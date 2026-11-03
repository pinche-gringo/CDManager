// PROJECT     : CDManager
// SUBSYSTEM   : Database
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 03.11.2026
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

#include <dlfcn.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>

#include <glibmm/ustring.h>

#include <YGP/Trace.h>

#include "DBFactory.h"

namespace {

/// Returns the passed message without trailing whitespace (libpq ends its messages with a newline)
std::string trim(std::string msg) {
    msg.erase(msg.find_last_not_of(" \t\r\n") + 1);
    return msg;
}

/// Returns the information about the backend with the passed name
const DBFactory::Backend* find(std::string_view name) {
    const auto* backend(std::ranges::find(DBFactory::backends, name, &DBFactory::Backend::name));
    return (backend == DBFactory::backends.end()) ? nullptr : backend;
}

} // namespace

//-----------------------------------------------------------------------------
/// Checks if the passed name specifies a backend (or the automatic detection)
/// \param backend Name of the backend
/// \returns bool True, if the name is valid
//-----------------------------------------------------------------------------
bool DBFactory::isValid(std::string_view backend) { return backend.empty() || (backend == AUTO) || find(backend); }

//-----------------------------------------------------------------------------
/// Returns the backends to try (in that order) to connect with the passed
/// parameters
/// \param backend Name of the backend to use; empty or AUTO to try all
/// \param params Parameters of the connection
/// \returns std::vector<const Backend*> Backends to try; empty if backend is invalid
//-----------------------------------------------------------------------------
std::vector<const DBFactory::Backend*> DBFactory::candidates(std::string_view backend, const ConnectParams& params) {
    if (backend.size() && (backend != AUTO)) {
        const Backend* chosen(find(backend));
        return chosen ? std::vector<const Backend*>{chosen} : std::vector<const Backend*>{};
    }

    std::vector<const Backend*> result;
    for (const Backend& candidate : backends)
        result.push_back(&candidate);

    // Prefer the backend matching the passed port or socket. On another port try PostgreSQL first: Its
    // client fails immediately at a MySQL server (which greets first), while a MySQL client waits for the
    // greeting of a PostgreSQL server until the timeout
    const Backend* preferred(nullptr);
    if (params.port) {
        const auto* match(std::ranges::find(backends, params.port, &Backend::defaultPort));
        preferred = (match == backends.end()) ? find("postgresql") : match;
    }
    else if (params.host.starts_with('/') && params.host.contains(".s.PGSQL."))
        preferred = find("postgresql");
    if (preferred)
        std::ranges::stable_partition(result, [preferred](const Backend* b) { return b == preferred; });
    return result;
}

//-----------------------------------------------------------------------------
/// Creates the database object of a backend by loading its module
/// \param backend Backend to create the object for
/// \returns std::unique_ptr<Database> Newly created (unconnected) database object
/// \throw std::runtime_error If the module can't be loaded
//-----------------------------------------------------------------------------
std::unique_ptr<Database> DBFactory::create(const Backend& backend) {
    const char* dir(std::getenv("CDMGR_MODULEDIR"));
    const std::filesystem::path module(std::filesystem::path(dir ? dir : PKGLIBDIR) / std::format("cdmgr-{}.so", backend.name));
    TRACE5("DBFactory::create (const Backend&) - Loading " << module);

    // The module is never unloaded, as it must outlive its database objects
    void* handle(dlopen(module.c_str(), RTLD_NOW | RTLD_LOCAL));
    if (!handle)
        throw std::runtime_error(dlerror());
    auto* createDB(reinterpret_cast<decltype(&cdmgr_createDatabase)>(dlsym(handle, "cdmgr_createDatabase")));
    if (!createDB)
        throw std::runtime_error(dlerror());
    return std::unique_ptr<Database>(createDB());
}

//-----------------------------------------------------------------------------
/// Connects to the database. Without a specified backend every installed one
/// is tried; the first accepting the connection is used. If none does, the
/// error of the first server answering is reported. If the port or socket is
/// specified, no further backend is tried after a server answered there.
/// \param backend Name of the backend to use; empty or AUTO to detect it
/// \param params Parameters of the connection
/// \param used Set to the backend of the returned database
/// \param creator Function creating the database object of a backend
/// \returns std::unique_ptr<Database> Connected database
/// \throw DBUnreachable If no server answered
/// \throw std::exception In case of another error
//-----------------------------------------------------------------------------
std::unique_ptr<Database> DBFactory::connect(std::string_view backend, const ConnectParams& params, const Backend*& used,
                                             const Creator& creator) {
    const auto tries(candidates(backend, params));
    if (tries.empty())
        throw std::invalid_argument(Glib::ustring::compose(_("Unknown database backend `%1'!"), std::string(backend)));

    // A single backend reports its errors unchanged
    if (tries.size() == 1) {
        std::unique_ptr<Database> db(creator(*tries[0]));
        db->connect(params);
        used = tries[0];
        return db;
    }

    std::string refused;  // Error of the first server answering
    std::string notFound; // Errors of the backends not finding a server
    std::string missing;  // Errors of the backends not installed
    for (const Backend* candidate : tries) {
        std::unique_ptr<Database> db;
        try {
            db = creator(*candidate);
        }
        catch (std::exception& err) {
            missing += std::format("\n{}: {}", candidate->displayName, trim(err.what()));
            continue;
        }

        try {
            TRACE3("DBFactory::connect () - Trying " << candidate->name);
            db->connect(params);
            used = candidate;
            return db;
        }
        catch (DBUnreachable& err) {
            notFound += std::format("\n{}: {}", candidate->displayName, trim(err.what()));
        }
        catch (std::exception& err) {
            if (refused.empty())
                refused = std::format("{}: {}", candidate->displayName, trim(err.what()));
            // A server answered at the explicitly specified address; other backends would only wait for the timeout there
            if (params.port || params.host.starts_with('/'))
                break;
        }
    }

    if (refused.size())
        throw std::runtime_error(refused);
    if (notFound.size())
        throw DBUnreachable(Glib::ustring(_("No database server found!")) + notFound + missing);
    throw std::runtime_error(Glib::ustring(_("No database backend is installed!")) + missing);
}
