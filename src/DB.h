#ifndef DB_H
#define DB_H

//$Id: DB.h,v 1.3 2005/01/18 03:56:56 markus Rel $

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


#include <string>
#include <stdexcept>


/**This class encapsulates all methods to access the database.

   It is not sovled very elegantly, but mysql++ defines a Row (to MysqlRow)
   which quite interferes with as Row-datatype (such as Gtk::Table's) and so
   the database-access was totally encapsulated.
 */
class Database {
 public:
   static void connect (const char* db, const char* user, const char* pwd)
      throw (std::exception&);
   static void close () throw (std::exception&);

   static void store (const char* query) throw (std::exception&);
   static unsigned int resultSize ();
   static bool hasData ();
   static void getNextResultRow ();
   static std::string getResultColumnAsString (unsigned int column);
   static unsigned int getResultColumnAsUInt (unsigned int column);
   static int getResultColumnAsInt (unsigned int column);
   static long getIDOfInsert ();

 private:
   Database ();
   Database (const Database& other);
   virtual ~Database ();

   const Database& operator= (const Database& other);
};

#endif
