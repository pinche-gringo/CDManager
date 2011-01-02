//PROJECT     : CDManager
//SUBSYSTEM   : Database
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 16.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2010

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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DB.h"


#if defined HAVE_LIBMYSQLPP
#  include <cstring>   // Only needed by mysql++.h (when compiled with GCC 4.3)
#  include <mysql++.h>

static mysqlpp::Connection       con (mysqlpp::use_exceptions);

#if MYSQLPP_HEADER_VERSION < 0x030000
   static mysqlpp::Result           result;
   static mysqlpp::Result::iterator i;
#else
   static mysqlpp::StoreQueryResult           result;
   static mysqlpp::StoreQueryResult::iterator i;

   static long lastIDInsert = 0;
#endif


void Database::connect (const char* db, const char* user, const char* pwd) throw (std::exception&) {
   TRACE9 ("Database::connect (const char* (3x) - " << db << " from " << user);
   con.connect (db, NULL, user, pwd);
}

void Database::close () throw (std::exception&) {
   TRACE9 ("Database::close ()");
#if MYSQLPP_HEADER_VERSION < 0x030000
   con.close ();
#else
   con.disconnect ();
#endif
}

bool Database::connected () {
   return con.connected ();
}

void Database::execute (const char* query) throw (std::exception&) {
   TRACE1 ("Database::execute (const char*) - " << query);
   mysqlpp::Query q (con.query ());
   result = q.store (query);
   i = result.begin ();

#if MYSQLPP_HEADER_VERSION >= 0x030000
   lastIDInsert = q.insert_id ();
#endif
}

unsigned int Database::resultSize () {
   TRACE9 ("Database::resultSize () - " << result.size ());
   return result.size ();
}

bool Database::hasData () {
   TRACE9 ("Database::hasData () - " << (i != result.end () ? "Yes" : "No"));
   return i != result.end ();
}

void Database::getNextResultRow () {
   TRACE9 ("Database::getNextResultRow ()");
   ++i;
}

std::string Database::getResultColumnAsString (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsString (unsigned int) - " << column);
   std::string ret ((*i)[column]);
   return ret;
}

unsigned int Database::getResultColumnAsUInt (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsUInt (unsigned int) - " << column);
   unsigned int ret ((*i)[column]);
   return ret;
}

int Database::getResultColumnAsInt (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsInt (unsigned int) - " << column);
   int ret ((*i)[column]);
   return ret;
}

long Database::getIDOfInsert () {
#if MYSQLPP_HEADER_VERSION < 0x030000
   return con.insert_id ();
#else
   return lastIDInsert;
#endif
}

//-----------------------------------------------------------------------------
/// Escapes the quotes in values for the database
/// \param value: Value to escape
/// \returns Glib::ustring: Escaped text
//-----------------------------------------------------------------------------
std::string Database::escapeDBValue (const std::string& value) {
   mysqlpp::Query q (con.query ());
   std::string conv;
   char* buffer = new char [(value.length () << 1) + 1];
   conv = std::string (buffer, q.escape_string (buffer, value.c_str (), value.length ()));
   delete [] buffer;
   return conv;
}


#elif defined HAVE_LIBMYSQL
#  include <cstdlib>

#  include <mysql.h>


static MYSQL* mysql (NULL);
static MYSQL_RES* result (NULL);
static MYSQL_ROW row (NULL);
static unsigned int sizeResult (0);


void Database::connect (const char* db, const char* user, const char* pwd) throw (std::exception&) {
   TRACE9 ("Database::connect (const char* (3x) - " << db << " from " << user);
   Check2 (!mysql);
   mysql = mysql_init (NULL);
   if (!mysql_real_connect (mysql, NULL, user, pwd, db, 0, NULL, 0)) {
      std::runtime_error error (mysql_error (mysql));
      close ();
      throw error;
   }
}

void Database::close () throw (std::exception&) {
   TRACE9 ("Database::close ()");
   mysql_close (mysql);
   mysql = NULL;
}

bool Database::connected () {
   return mysql != NULL;
}

void Database::execute (const char* query) throw (std::exception&) {
   TRACE1 ("Database::execute (const char*) - " << query);
   if (mysql_query (mysql, query))
      throw std::runtime_error (mysql_error (mysql));

   if (!result)
      mysql_free_result (result);

   if ((result = mysql_store_result (mysql)) != NULL) {
      row = mysql_fetch_row (result);
      sizeResult = mysql_num_rows (result);
   }
   else {
      if (mysql_errno (mysql))
	 throw std::runtime_error (mysql_error (mysql));
      sizeResult = 0;
   }
}

unsigned int Database::resultSize () {
   TRACE9 ("Database::resultSize () - " << sizeResult);
   return sizeResult;
}

bool Database::hasData () {
   TRACE9 ("Database::hasData () - " << (row ? "Yes" : "No"));
   return row;
}

void Database::getNextResultRow () {
   TRACE9 ("Database::getNextResultRow ()");
   row = mysql_fetch_row (result);
   if (!row) {
      mysql_free_result (result);
      result = NULL;
   }
}

std::string Database::getResultColumnAsString (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsString (unsigned int) - " << column);
   Check2 (result); Check2 (row);
   Check1 (column < mysql_num_fields (result));
   return row[column];
}

unsigned int Database::getResultColumnAsUInt (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsUInt (unsigned int) - " << column);
   TRACE9 ("Database::getResultColumnAsUInt (unsigned int) - " << row[column]);
   Check2 (result); Check2 (row);
   Check1 (column < mysql_num_fields (result));
   return strtoul (row[column], NULL, 10);
}

int Database::getResultColumnAsInt (unsigned int column) {
   TRACE9 ("Database::getResultColumnAsInt (unsigned int) - " << column);
   Check2 (result); Check2 (row);
   Check1 (column < mysql_num_fields (result));
   return strtol (row[column], NULL, 10);
}

long Database::getIDOfInsert () {
   return mysql_insert_id (mysql);
}

//-----------------------------------------------------------------------------
/// Escapes the quotes in values for the database
/// \param value: Value to escape
/// \returns Glib::ustring: Escaped text
//-----------------------------------------------------------------------------
std::string Database::escapeDBValue (const std::string& value) {
   std::string conv;
   char* buffer = new char [(value.length () << 1) + 1];
   conv = std::string (buffer, mysql_real_escape_string (mysql, buffer, value.c_str (), value.length ()));
   delete [] buffer;
   return conv;
}

#else
#  error No supported database detected!
#endif
