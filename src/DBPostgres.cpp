//PROJECT     : CDManager
//SUBSYSTEM   : Database/PostgreSQL
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 25.09.2026
//COPYRIGHT   : Copyright (C) 2026

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

#include <cstdlib>
#include <memory>

#include <libpq-fe.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DBPostgres.h"


static const Oid BYTEAOID (17);          ///< Type-ID of bytea (from pg_type.h)

/// Frees a PGresult, when going out of scope
typedef std::unique_ptr<PGresult, void (*) (PGresult*)> PResult;


//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
DBPostgres::DBPostgres () : conn (NULL) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DBPostgres::~DBPostgres () {
   close ();
}

//-----------------------------------------------------------------------------
/// Connects to the database. The server is specified by the usual libpq
/// environment variables (like PGHOST, PGPORT); default is the local host.
/// \param db Name of database
/// \param user User to use for the DB
/// \param pwd Password of user
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void DBPostgres::connect (const char* db, const char* user, const char* pwd) {
   TRACE9 ("DBPostgres::connect (const char* (3x) - " << db << " from " << user);
   Check2 (!conn);

   const char* keys[] = { "hostaddr", "dbname", "user", "password", "client_encoding", NULL };
   const char* values[] = { "127.0.0.1", db, user, pwd, "UTF8", NULL };
   conn = PQconnectdbParams (keys, values, 0);
   if (!conn)
      throw std::runtime_error ("Out of memory initialising PostgreSQL");
   if (PQstatus (conn) != CONNECTION_OK) {
      std::runtime_error error (PQerrorMessage (conn));
      close ();
      throw error;
   }
}

//-----------------------------------------------------------------------------
/// Closes the connection to the database
//-----------------------------------------------------------------------------
void DBPostgres::close () {
   TRACE9 ("DBPostgres::close ()");
   if (conn) {
      PQfinish (conn);
      conn = NULL;
   }
}

//-----------------------------------------------------------------------------
/// Checks if the connection to the database is established
/// \returns bool True, if connected
//-----------------------------------------------------------------------------
bool DBPostgres::connected () const {
   return conn != NULL;
}

//-----------------------------------------------------------------------------
/// Executes the passed query. Columns of type bytea are returned unescaped.
/// \param query Query to execute
/// \param result Vector receiving the returned rows
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void DBPostgres::query (const char* query, std::vector<Row>& result) {
   Check2 (conn);
   PResult res (PQexec (conn, query), PQclear);
   switch (PQresultStatus (res.get ())) {
   case PGRES_COMMAND_OK:
      return;

   case PGRES_TUPLES_OK:
      break;

   default:
      throw std::runtime_error (res ? PQresultErrorMessage (res.get ()) : PQerrorMessage (conn));
   }

   int cRows (PQntuples (res.get ()));
   int cColumns (PQnfields (res.get ()));
   std::vector<bool> binary (cColumns);
   for (int i (0); i < cColumns; ++i)
      binary[i] = PQftype (res.get (), i) == BYTEAOID;

   result.reserve (cRows);
   for (int r (0); r < cRows; ++r) {
      result.push_back (Row ());
      Row& target (result.back ());
      target.reserve (cColumns);
      for (int c (0); c < cColumns; ++c) {
	 const char* value (PQgetvalue (res.get (), r, c));
	 if (PQgetisnull (res.get (), r, c))
	    target.push_back (std::string ());
	 else if (binary[c]) {
	    size_t len (0);
	    unsigned char* data (PQunescapeBytea (reinterpret_cast<const unsigned char*> (value), &len));
	    if (!data)
	       throw std::runtime_error ("Out of memory decoding binary data");
	    target.push_back (std::string (reinterpret_cast<char*> (data), len));
	    PQfreemem (data);
	 }
	 else
	    target.push_back (std::string (value, PQgetlength (res.get (), r, c)));
      }
   }
}

//-----------------------------------------------------------------------------
/// Returns the ID generated by the last INSERT (the last value of a sequence
/// used in this session)
/// \returns long Generated ID
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
long DBPostgres::getIDOfInsert () {
   Check2 (conn);
   PResult res (PQexec (conn, "SELECT lastval()"), PQclear);
   if ((PQresultStatus (res.get ()) != PGRES_TUPLES_OK) || (PQntuples (res.get ()) != 1))
      throw std::runtime_error (res ? PQresultErrorMessage (res.get ()) : PQerrorMessage (conn));
   return strtol (PQgetvalue (res.get (), 0, 0), NULL, 10);
}

//-----------------------------------------------------------------------------
/// Escapes the passed text for the use inside a SQL string
/// \param value Text to escape
/// \returns std::string Escaped text
/// \throw std::exception In case of an error (e.g. invalid encoding)
//-----------------------------------------------------------------------------
std::string DBPostgres::escapeDBValue (const std::string& value) const {
   Check2 (conn);
   std::string conv ((value.length () << 1) + 1, '\0');
   int error (0);
   conv.resize (PQescapeStringConn (conn, &conv[0], value.data (), value.length (), &error));
   if (error)
      throw std::runtime_error (PQerrorMessage (conn));
   return conv;
}

//-----------------------------------------------------------------------------
/// Returns the passed binary data as SQL literal (for a bytea-column)
/// \param value Data to quote
/// \returns std::string SQL literal
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
std::string DBPostgres::quoteBlob (const std::string& value) const {
   Check2 (conn);
   size_t len (0);
   unsigned char* data (PQescapeByteaConn (conn, reinterpret_cast<const unsigned char*> (value.data ()),
					  value.length (), &len));
   if (!data)
      throw std::runtime_error (PQerrorMessage (conn));
   std::string conv ('\'' + std::string (reinterpret_cast<char*> (data)) + '\'');
   PQfreemem (data);
   return conv;
}
