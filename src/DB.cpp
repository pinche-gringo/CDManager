//$Id: DB.cpp,v 1.2 2004/11/12 03:56:48 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Database
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
//AUTHOR      : Markus Schwab
//CREATED     : 16.10.2004
//COPYRIGHT   : Anticopyright (A) 2004

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

#ifdef HAVE_LIBMYSQLPP
#  include <mysql++.hh>
#else
#  error No supported database detected!
#endif

#define CHECK 9
#define TRACELEVEL 5
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DB.h"

static Connection       con (use_exceptions);
static Result           result;
static Result::iterator i;


void Database::connect (const char* db, const char* user, const char* pwd) throw (std::exception&) {
   TRACE9 ("Database::connect (const char* (3x) - " << db << " from " << user);
   con.connect (db, NULL, user, pwd);
}

void Database::close () throw (std::exception&) {
   TRACE9 ("Database::close ()");
   con.close ();
}

void Database::store (const char* query) throw (std::exception&) {
   TRACE1 ("Database::store (const char*) - " << query);
   Query q (con.query ());
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
