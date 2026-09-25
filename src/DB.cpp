//PROJECT     : CDManager
//SUBSYSTEM   : Database
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 16.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2010, 2011, 2026

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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DB.h"

#if defined HAVE_LIBPQ
#  include "DBPostgres.h"
#elif defined HAVE_LIBMYSQL
#  include "DBMySQL.h"
#else
#  error No supported database detected!
#endif


//-----------------------------------------------------------------------------
/// Creates the database-object for the database configured at compile-time
/// \returns Database* Newly created object; free it with delete
//-----------------------------------------------------------------------------
Database* Database::create () {
#if defined HAVE_LIBPQ
   return new DBPostgres;
#else
   return new DBMySQL;
#endif
}

//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
Database::Database () : current (0) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Database::~Database () {
}

//-----------------------------------------------------------------------------
/// Executes the passed query; its result can be accessed afterwards
/// \param query Query to execute
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void Database::execute (const char* query) {
   TRACE1 ("Database::execute (const char*) - " << query);
   Check1 (query);
   Check2 (connected ());

   rows.clear ();
   current = 0;
   this->query (query, rows);
}

//-----------------------------------------------------------------------------
/// Inserts a new row into a table
/// \param table Table to insert into
/// \param values Columns and values of the new row
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void Database::insert (const char* table, const Values& values) {
   Check1 (table); Check1 (!values.empty ());

   std::string columns, data;
   for (Values::const_iterator i (values.begin ()); i != values.end (); ++i) {
      if (i != values.begin ()) {
	 columns += ", ";
	 data += ", ";
      }
      columns += i->first;
      data += i->second;
   }
   execute (std::string ("INSERT INTO ") + table + " (" + columns + ") VALUES (" + data + ')');
}

//-----------------------------------------------------------------------------
/// Updates the rows of a table
/// \param table Table to update
/// \param values Columns and their new values
/// \param where Condition selecting the rows to update
/// \throw std::exception In case of an error
//-----------------------------------------------------------------------------
void Database::update (const char* table, const Values& values, const std::string& where) {
   Check1 (table); Check1 (!values.empty ()); Check1 (where.size ());

   std::string cmd (std::string ("UPDATE ") + table + " SET ");
   for (Values::const_iterator i (values.begin ()); i != values.end (); ++i) {
      if (i != values.begin ())
	 cmd += ", ";
      cmd += i->first + '=' + i->second;
   }
   execute (cmd + " WHERE " + where);
}

//-----------------------------------------------------------------------------
/// Returns the passed text as quoted SQL string
/// \param value Text to quote
/// \returns std::string Escaped text surrounded by single quotes
//-----------------------------------------------------------------------------
std::string Database::quote (const std::string& value) const {
   return '\'' + escapeDBValue (value) + '\'';
}

//-----------------------------------------------------------------------------
/// Returns the passed column of the actual row of the result
/// \param column Index of column
/// \returns const std::string& Value of column
//-----------------------------------------------------------------------------
const std::string& Database::column (unsigned int column) const {
   Check2 (hasData ());
   Check1 (column < rows[current].size ());
   return rows[current][column];
}

const std::string& Database::getResultColumnAsBlob (unsigned int column) const {
   TRACE9 ("Database::getResultColumnAsBlob (unsigned int) - " << column);
   return this->column (column);
}

const std::string& Database::getResultColumnAsString (unsigned int column) const {
   TRACE9 ("Database::getResultColumnAsString (unsigned int) - " << column);
   return this->column (column);
}

unsigned int Database::getResultColumnAsUInt (unsigned int column) const {
   TRACE9 ("Database::getResultColumnAsUInt (unsigned int) - " << column);
   return strtoul (this->column (column).c_str (), NULL, 10);
}

int Database::getResultColumnAsInt (unsigned int column) const {
   TRACE9 ("Database::getResultColumnAsInt (unsigned int) - " << column);
   return strtol (this->column (column).c_str (), NULL, 10);
}
