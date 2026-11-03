// PROJECT     : CDManager
// SUBSYSTEM   : Database/PostgreSQL
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 25.09.2026
// COPYRIGHT   : Copyright (C) 2026

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.

#include <cdmgr-cfg.h>

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <libpq-fe.h>

#include <YGP/Trace.h>

#include "DBPostgres.h"

constexpr Oid BYTEAOID(17); ///< Type-ID of bytea (from pg_type.h)

/// Connection attempts give up after this time (in seconds)
constexpr const char* CONNECT_TIMEOUT("10");
/// Timeout for checking if a server answers after a failed connection; it
/// answers fast, if it just did
constexpr const char* PING_TIMEOUT("2");
/// Start of the name of the socket of a PostgreSQL server (followed by the port)
constexpr std::string_view SOCKET_PREFIX(".s.PGSQL.");

/// Frees a PGresult, when going out of scope
using PResult = std::unique_ptr<PGresult, void (*)(PGresult*)>;

//-----------------------------------------------------------------------------
/// Creates the database object; exported for DBFactory
/// \returns Database* Newly created object; free it with delete
//-----------------------------------------------------------------------------
Database* cdmgr_createDatabase() { return new DBPostgres; }

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DBPostgres::~DBPostgres() { close(); }

//-----------------------------------------------------------------------------
/// Connects to the database; without host on the local host (via TCP/IP).
/// The host can also be the directory containing the socket or the socket
/// itself (like /run/postgresql/.s.PGSQL.5432).
/// \param params Parameters of the connection
/// \throw DBUnreachable If no PostgreSQL server answers
/// \throw std::exception In case of another error
//-----------------------------------------------------------------------------
void DBPostgres::connect(const ConnectParams& params) {
    TRACE9("DBPostgres::connect (const ConnectParams&) - " << params.database << " from " << params.user << '@' << params.host
                                                           << ':' << params.port);
    contract_assert(!conn);

    // PostgreSQL folds unquoted names to lower case (CREATE DATABASE CDMedia creates cdmedia),
    // but compares the name passed when connecting exactly; so connect to the folded name
    const std::string dbName(params.database | std::views::transform([](unsigned char c) { return std::tolower(c); }) |
                             std::ranges::to<std::string>());

    std::string host(params.host);
    std::string port(params.port ? std::to_string(params.port) : std::string());
    if (host.starts_with('/')) {
        // libpq expects the directory of the socket; its name contains the port
        const std::filesystem::path socket(host);
        const std::string name(socket.filename());
        if (name.starts_with(SOCKET_PREFIX)) {
            host = socket.parent_path();
            if (port.empty())
                port = name.substr(SOCKET_PREFIX.size());
        }
    }

    std::vector<const char*> keys{"connect_timeout", "dbname", "user", "password", "client_encoding"};
    std::vector<const char*> values{CONNECT_TIMEOUT, dbName.c_str(), params.user.c_str(), params.password.c_str(), "UTF8"};
    keys.push_back(host.empty() ? "hostaddr" : "host");
    values.push_back(host.empty() ? "127.0.0.1" : host.c_str());
    if (port.size()) {
        keys.push_back("port");
        values.push_back(port.c_str());
    }
    keys.push_back(nullptr);
    values.push_back(nullptr);

    conn = PQconnectdbParams(keys.data(), values.data(), 0);
    if (!conn)
        throw std::runtime_error("Out of memory initialising PostgreSQL");
    if (PQstatus(conn) != CONNECTION_OK) {
        const std::string msg(PQerrorMessage(conn));
        close();
        // Ping only reports a server, if it answered as PostgreSQL (even if rejecting the login)
        values.front() = PING_TIMEOUT;
        if (PQpingParams(keys.data(), values.data(), 0) == PQPING_NO_RESPONSE)
            throw DBUnreachable(msg);
        throw std::runtime_error(msg);
    }
}

//-----------------------------------------------------------------------------
/// Closes the connection to the database
//-----------------------------------------------------------------------------
void DBPostgres::close() {
    TRACE9("DBPostgres::close ()");
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

//-----------------------------------------------------------------------------
/// Checks if the connection to the database is established
/// \returns bool True, if connected
//-----------------------------------------------------------------------------
bool DBPostgres::connected() const { return conn != nullptr; }

//-----------------------------------------------------------------------------
/// Executes the passed query. Columns of type bytea are returned unescaped.
/// \param query Query to execute
/// \param result Vector receiving the returned rows
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void DBPostgres::query(const char* query, std::vector<Row>& result) {
    contract_assert(conn);
    PResult res(PQexec(conn, query), PQclear);
    switch (PQresultStatus(res.get())) {
    case PGRES_COMMAND_OK:
        return;

    case PGRES_TUPLES_OK:
        break;

    default:
        throw std::runtime_error(res ? PQresultErrorMessage(res.get()) : PQerrorMessage(conn));
    }

    const int cRows(PQntuples(res.get()));
    const int cColumns(PQnfields(res.get()));
    const auto binary(std::views::iota(0, cColumns) |
                      std::views::transform([&res](int c) { return PQftype(res.get(), c) == BYTEAOID; }) |
                      std::ranges::to<std::vector<bool>>());

    result.reserve(cRows);
    for (int r(0); r < cRows; ++r) {
        Row& target(result.emplace_back());
        target.reserve(cColumns);
        for (int c(0); c < cColumns; ++c) {
            const char* value(PQgetvalue(res.get(), r, c));
            if (PQgetisnull(res.get(), r, c))
                target.emplace_back();
            else if (binary[c]) {
                size_t len(0);
                std::unique_ptr<unsigned char, void (*)(void*)> data(
                    PQunescapeBytea(reinterpret_cast<const unsigned char*>(value), &len), PQfreemem);
                if (!data)
                    throw std::runtime_error("Out of memory decoding binary data");
                target.emplace_back(reinterpret_cast<const char*>(data.get()), len);
            }
            else
                target.emplace_back(value, PQgetlength(res.get(), r, c));
        }
    }
}

//-----------------------------------------------------------------------------
/// Returns the ID generated by the last INSERT (the last value of a sequence
/// used in this session)
/// \returns long Generated ID
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
long DBPostgres::getIDOfInsert() {
    contract_assert(conn);
    PResult res(PQexec(conn, "SELECT lastval()"), PQclear);
    if ((PQresultStatus(res.get()) != PGRES_TUPLES_OK) || (PQntuples(res.get()) != 1))
        throw std::runtime_error(res ? PQresultErrorMessage(res.get()) : PQerrorMessage(conn));
    return std::strtol(PQgetvalue(res.get(), 0, 0), nullptr, 10);
}

//-----------------------------------------------------------------------------
/// Escapes the passed text for the use inside a SQL string
/// \param value Text to escape
/// \returns std::string Escaped text
/// \throw std::exception In case of an error (e.g. invalid encoding)
//-----------------------------------------------------------------------------
std::string DBPostgres::escapeDBValue(const std::string& value) const {
    contract_assert(conn);
    std::string conv((value.length() << 1) + 1, '\0');
    int error(0);
    conv.resize(PQescapeStringConn(conn, conv.data(), value.data(), value.length(), &error));
    if (error)
        throw std::runtime_error(PQerrorMessage(conn));
    return conv;
}

//-----------------------------------------------------------------------------
/// Returns the passed binary data as SQL literal (for a bytea-column)
/// \param value Data to quote
/// \returns std::string SQL literal
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
std::string DBPostgres::quoteBlob(const std::string& value) const {
    contract_assert(conn);
    size_t len(0);
    std::unique_ptr<unsigned char, void (*)(void*)> data(
        PQescapeByteaConn(conn, reinterpret_cast<const unsigned char*>(value.data()), value.length(), &len), PQfreemem);
    if (!data)
        throw std::runtime_error(PQerrorMessage(conn));
    return std::format("'{}'", reinterpret_cast<const char*>(data.get()));
}
