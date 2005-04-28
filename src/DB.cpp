//$Id: DB.cpp,v 1.6 2005/04/28 19:36:59 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Database
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.6 $
//AUTHOR      : Markus Schwab
//CREATED     : 16.10.2004
//COPYRIGHT   : Copyright (C) 2004, 2005

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <cdmgr-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DB.h"

#if defined HAVE_LIBMYSQLPP
#  include <mysql++.h>

static mysqlpp::Connection       con (mysqlpp::use_exceptions);
static mysqlpp::Result           result;
static mysqlpp::Result::iterator i;


void Database::connect (const char* db, const char* user, const char* pwd) throw (std::exception&) {
   TRACE9 ("Database::connect (const char* (3x) - " << db << " from " << user);
   con.connect (db, NULL, user, pwd);
}

void Database::close () throw (std::exception&) {
   TRACE9 ("Database::close ()");
   con.close ();
}

void Database::execute (const char* query) throw (std::exception&) {
   TRACE1 ("Database::execute (const char*) - " << query);
   mysqlpp::Query q (con.query ());
   result = q.store (query);
   i = result.begin ();
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
   return con.insert_id ();
}

#elif defined HAVE_LIBMYSQL
#  include <cstdlib>

#  include <mysql.h>


static MYSQL* mysql (NULL);
static MYSQL_RES* result (NULL);
static MYSQL_ROW row (NULL);


void Database::connect (const char* db, const char* user, const char* pwd) throw (std::exception&) {
   TRACE9 ("Database::connect (const char* (3x) - " << db << " from " << user);
   Check2 (!mysql);
   mysql = mysql_init (NULL);
   if (!mysql_real_connect (mysql, NULL, user, pwd, db, 0, NULL, 0))
      throw std::runtime_error (mysql_error (mysql));
}

void Database::close () throw (std::exception&) {
   TRACE9 ("Database::close ()");
   mysql_close (mysql);
}

void Database::execute (const char* query) throw (std::exception&) {
   TRACE1 ("Database::execute (const char*) - " << query);
   if (mysql_query (mysql, query))
      throw std::runtime_error (mysql_error (mysql));

   if (!result)
      mysql_free_result (result);

   if ((result = mysql_store_result (mysql)) != NULL)
      row = mysql_fetch_row (result);
   else
      if (mysql_errno (mysql))
	 throw std::runtime_error (mysql_error (mysql));
}

unsigned int Database::resultSize () {
   TRACE9 ("Database::resultSize () - " << mysql_num_rows (result));
   return mysql_num_rows (result);
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

#else
#  error No supported database detected!
#endif
