#ifndef DBMYSQL_H
#define DBMYSQL_H

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

using MYSQL = struct st_mysql;

/**Access to a MySQL (or MariaDB) database via the MySQL C API
 */
class DBMySQL : public Database {
  public:
    DBMySQL() = default;
    ~DBMySQL() override;

    void connect(const char* db, const char* user, const char* pwd) override;
    void close() override;
    [[nodiscard]] bool connected() const override;

    long getIDOfInsert() override;

    [[nodiscard]] std::string escapeDBValue(const std::string& value) const override;
    [[nodiscard]] std::string quoteBlob(const std::string& value) const override;

  protected:
    void query(const char* query, std::vector<Row>& result) override;

  private:
    MYSQL* mysql{nullptr};
};

#endif
