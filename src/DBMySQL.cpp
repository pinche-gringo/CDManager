// PROJECT     : CDManager
// SUBSYSTEM   : Database/MySQL
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

#include <cstring>
#include <memory>
#include <string>

#include <errmsg.h>
#include <mysql.h>

#include <YGP/Trace.h>

#include "DBMySQL.h"

/// Connection attempts give up after this time (in seconds); so a server not
/// speaking the MySQL protocol (which waits for the client) is detected
constexpr unsigned int CONNECT_TIMEOUT(10);

//-----------------------------------------------------------------------------
/// Creates the database object; exported for DBFactory
/// \returns Database* Newly created object; free it with delete
//-----------------------------------------------------------------------------
Database* cdmgr_createDatabase() { return new DBMySQL; }

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DBMySQL::~DBMySQL() { close(); }

//-----------------------------------------------------------------------------
/// Connects to the database; without host on the local host (via the default
/// socket)
/// \param params Parameters of the connection
/// \throw DBUnreachable If no MySQL server answers
/// \throw std::exception In case of another error
//-----------------------------------------------------------------------------
void DBMySQL::connect(const ConnectParams& params) {
    TRACE9("DBMySQL::connect (const ConnectParams&) - " << params.database << " from " << params.user << '@' << params.host << ':'
                                                        << params.port);
    contract_assert(!mysql);

    mysql = mysql_init(nullptr);
    if (!mysql)
        throw std::runtime_error("Out of memory initialising MySQL");
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &CONNECT_TIMEOUT);

    const bool socket(params.host.starts_with('/'));
    const char* host((params.host.empty() || socket) ? nullptr : params.host.c_str());
    if (!mysql_real_connect(mysql, host, params.user.c_str(), params.password.c_str(), params.database.c_str(), params.port,
                            socket ? params.host.c_str() : nullptr, 0)) {
        const unsigned int code(mysql_errno(mysql));
        const std::string msg(mysql_error(mysql));
        close();
        switch (code) {
        case CR_CONNECTION_ERROR:
        case CR_CONN_HOST_ERROR:
        case CR_UNKNOWN_HOST:
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_HANDSHAKE_ERR:
        case CR_SERVER_LOST:
        case CR_MALFORMED_PACKET:
            throw DBUnreachable(msg);

        default:
            throw std::runtime_error(msg);
        }
    }
}

//-----------------------------------------------------------------------------
/// Closes the connection to the database
//-----------------------------------------------------------------------------
void DBMySQL::close() {
    TRACE9("DBMySQL::close ()");
    if (mysql) {
        mysql_close(mysql);
        mysql = nullptr;
    }
}

//-----------------------------------------------------------------------------
/// Checks if the connection to the database is established
/// \returns bool True, if connected
//-----------------------------------------------------------------------------
bool DBMySQL::connected() const { return mysql != nullptr; }

//-----------------------------------------------------------------------------
/// Executes the passed query
/// \param query Query to execute
/// \param result Vector receiving the returned rows
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void DBMySQL::query(const char* query, std::vector<Row>& result) {
    contract_assert(mysql);
    if (mysql_real_query(mysql, query, std::strlen(query)))
        throw std::runtime_error(mysql_error(mysql));

    std::unique_ptr<MYSQL_RES, void (*)(MYSQL_RES*)> res(mysql_store_result(mysql), mysql_free_result);
    if (!res) {
        if (mysql_field_count(mysql)) // Query should have returned data
            throw std::runtime_error(mysql_error(mysql));
        return;
    }

    const unsigned int cColumns(mysql_num_fields(res.get()));
    result.reserve(mysql_num_rows(res.get()));
    while (MYSQL_ROW row = mysql_fetch_row(res.get())) {
        const unsigned long* lengths(mysql_fetch_lengths(res.get()));
        Row& target(result.emplace_back());
        target.reserve(cColumns);
        for (unsigned int i(0); i < cColumns; ++i)
            target.push_back(row[i] ? std::string(row[i], lengths[i]) : std::string());
    }
}

//-----------------------------------------------------------------------------
/// Returns the ID generated by the last INSERT
/// \returns long Generated ID
//-----------------------------------------------------------------------------
long DBMySQL::getIDOfInsert() {
    contract_assert(mysql);
    return mysql_insert_id(mysql);
}

//-----------------------------------------------------------------------------
/// Escapes the passed text for the use inside a SQL string
/// \param value Text to escape
/// \returns std::string Escaped text
//-----------------------------------------------------------------------------
std::string DBMySQL::escapeDBValue(const std::string& value) const {
    contract_assert(mysql);
    std::string conv((value.length() << 1) + 1, '\0');
    conv.resize(mysql_real_escape_string(mysql, conv.data(), value.data(), value.length()));
    return conv;
}

//-----------------------------------------------------------------------------
/// Returns the passed binary data as SQL literal
/// \param value Data to quote
/// \returns std::string SQL literal
/// \remarks MySQL stores binary data in BLOBs like text
//-----------------------------------------------------------------------------
std::string DBMySQL::quoteBlob(const std::string& value) const { return quote(value); }
