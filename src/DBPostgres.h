#ifndef DBPOSTGRES_H
#define DBPOSTGRES_H


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


#include "DB.h"


typedef struct pg_conn PGconn;


/**Access to a PostgreSQL database via libpq
 */
class DBPostgres : public Database {
 public:
   DBPostgres ();
   virtual ~DBPostgres ();

   virtual void connect (const char* db, const char* user, const char* pwd);
   virtual void close ();
   virtual bool connected () const;

   virtual long getIDOfInsert ();

   virtual std::string escapeDBValue (const std::string& value) const;
   virtual std::string quoteBlob (const std::string& value) const;

 protected:
   virtual void query (const char* query, std::vector<Row>& result);

 private:
   PGconn* conn;
};

#endif
