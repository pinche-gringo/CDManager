#ifndef DBFACTORY_H
#define DBFACTORY_H

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

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "DB.h"

/**Creates the database objects of the backends, which are loaded at runtime
   from modules (named cdmgr-<backend>.so) in the module directory (PKGLIBDIR;
   can be overridden with the environment variable CDMGR_MODULEDIR).

   If no backend is specified, every installed one is tried until a server
   accepts the connection.
 */
class DBFactory {
  public:
    /// Information about a supported backend
    struct Backend {
        std::string_view name;        ///< Name of the backend (also part of the module name)
        std::string_view displayName; ///< Name to show to the user
        unsigned int defaultPort;     ///< Port the server listens on by default
    };
    static constexpr std::array<Backend, 2> backends{{{"mysql", "MySQL/MariaDB", 3306}, {"postgresql", "PostgreSQL", 5432}}};
    /// Name selecting the automatic detection of the backend
    static constexpr std::string_view AUTO{"auto"};

    /// Creates the (unconnected) database object for a backend
    using Creator = std::function<std::unique_ptr<Database>(const Backend&)>;

    [[nodiscard]] static bool isValid(std::string_view backend);
    [[nodiscard]] static std::vector<const Backend*> candidates(std::string_view backend, const ConnectParams& params);
    [[nodiscard]] static std::unique_ptr<Database> create(const Backend& backend);
    [[nodiscard]] static std::unique_ptr<Database> connect(std::string_view backend, const ConnectParams& params,
                                                           const Backend*& used, const Creator& creator = create);

  private:
    DBFactory() = delete;
};

#endif
