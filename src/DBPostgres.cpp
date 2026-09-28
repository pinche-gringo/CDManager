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

#include <array>
#include <cstdlib>
#include <format>
#include <memory>
#include <ranges>
#include <vector>

#include <libpq-fe.h>

#include <YGP/Trace.h>

#include "DBPostgres.h"

constexpr Oid BYTEAOID(17); ///< Type-ID of bytea (from pg_type.h)

/// Frees a PGresult, when going out of scope
using PResult = std::unique_ptr<PGresult, void (*)(PGresult*)>;

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DBPostgres::~DBPostgres() { close(); }

//-----------------------------------------------------------------------------
/// Connects to the database. The server is specified by the usual libpq
/// environment variables (like PGHOST, PGPORT); default is the local host.
/// \param db Name of database
/// \param user User to use for the DB
/// \param pwd Password of user
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void DBPostgres::connect(const char* db, const char* user, const char* pwd) {
    TRACE9("DBPostgres::connect (const char* (3x) - " << db << " from " << user);
    contract_assert(!conn);

    const std::array<const char*, 6> keys{"hostaddr", "dbname", "user", "password", "client_encoding", nullptr};
    const std::array<const char*, 6> values{"127.0.0.1", db, user, pwd, "UTF8", nullptr};
    conn = PQconnectdbParams(keys.data(), values.data(), 0);
    if (!conn)
        throw std::runtime_error("Out of memory initialising PostgreSQL");
    if (PQstatus(conn) != CONNECTION_OK) {
        std::runtime_error error(PQerrorMessage(conn));
        close();
        throw error;
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
