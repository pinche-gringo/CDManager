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

#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "DBFactory.h"

#define BOOST_TEST_MODULE DBFactory
#include "UnitTest.h"

namespace {

/// How a fake backend reacts on connecting
enum class Server { MISSING, NONE, REFUSING, ACCEPTING };

/// Database failing or succeeding to connect as specified
class FakeDatabase : public Database {
  public:
    explicit FakeDatabase(Server server) : server(server) {}

    void connect(const ConnectParams&) override {
        if (server == Server::NONE)
            throw DBUnreachable("Can't connect");
        if (server == Server::REFUSING)
            throw std::runtime_error("Access denied");
        isConnected = true;
    }
    void close() override { isConnected = false; }
    [[nodiscard]] bool connected() const override { return isConnected; }

    long getIDOfInsert() override { return 0; }
    [[nodiscard]] std::string escapeDBValue(const std::string& value) const override { return value; }
    [[nodiscard]] std::string quoteBlob(const std::string& value) const override { return value; }

  protected:
    void query(const char*, std::vector<Row>&) override {}

  private:
    Server server;
    bool isConnected{false};
};

/// Creates fake backends, recording the order of their creation
struct Fakes {
    DBFactory::Creator creator() {
        return [this](const DBFactory::Backend& backend) -> std::unique_ptr<Database> {
            tried.emplace_back(backend.name);
            const Server server(servers[std::string(backend.name)]);
            if (server == Server::MISSING)
                throw std::runtime_error("Module not found");
            return std::make_unique<FakeDatabase>(server);
        };
    }

    std::map<std::string, Server> servers;
    std::vector<std::string> tried;
};

/// Returns the names of the backends to try
std::vector<std::string> names(std::string_view backend, const ConnectParams& params) {
    std::vector<std::string> result;
    for (const auto* candidate : DBFactory::candidates(backend, params))
        result.emplace_back(candidate->name);
    return result;
}

using Names = std::vector<std::string>;

} // namespace

BOOST_AUTO_TEST_SUITE(Candidates)

BOOST_AUTO_TEST_CASE(Valid) {
    BOOST_TEST(DBFactory::isValid(""));
    BOOST_TEST(DBFactory::isValid("auto"));
    BOOST_TEST(DBFactory::isValid("mysql"));
    BOOST_TEST(DBFactory::isValid("postgresql"));
    BOOST_TEST(!DBFactory::isValid("oracle"));
}

BOOST_AUTO_TEST_CASE(Explicit) {
    BOOST_TEST(names("postgresql", {.port = 3306}) == Names{"postgresql"}, boost::test_tools::per_element());
    BOOST_TEST(names("mysql", {}) == Names{"mysql"}, boost::test_tools::per_element());
    BOOST_TEST(names("oracle", {}).empty());
}

BOOST_AUTO_TEST_CASE(DefaultOrder) {
    BOOST_TEST(names("", {}) == (Names{"mysql", "postgresql"}), boost::test_tools::per_element());
    BOOST_TEST(names("auto", {.host = "db.example.com"}) == (Names{"mysql", "postgresql"}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(PreferMatchingPort) {
    BOOST_TEST(names("auto", {.port = 3306}) == (Names{"mysql", "postgresql"}), boost::test_tools::per_element());
    BOOST_TEST(names("auto", {.port = 5432}) == (Names{"postgresql", "mysql"}), boost::test_tools::per_element());
    // PostgreSQL fails fast on a foreign server
    BOOST_TEST(names("auto", {.port = 4711}) == (Names{"postgresql", "mysql"}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(PreferMatchingSocket) {
    BOOST_TEST(names("auto", {.host = "/run/postgresql/.s.PGSQL.5432"}) == (Names{"postgresql", "mysql"}),
               boost::test_tools::per_element());
    BOOST_TEST(names("auto", {.host = "/var/lib/mysql/mysql.sock"}) == (Names{"mysql", "postgresql"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Connect)

BOOST_AUTO_TEST_CASE(FirstAccepting) {
    Fakes fakes{.servers = {{"mysql", Server::NONE}, {"postgresql", Server::ACCEPTING}}};
    const DBFactory::Backend* used(nullptr);
    auto db(DBFactory::connect("auto", {}, used, fakes.creator()));
    BOOST_TEST(db->connected());
    BOOST_REQUIRE(used);
    BOOST_TEST(used->name == "postgresql");
    BOOST_TEST(fakes.tried == (Names{"mysql", "postgresql"}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(StopsAtAccepting) {
    Fakes fakes{.servers = {{"mysql", Server::ACCEPTING}, {"postgresql", Server::ACCEPTING}}};
    const DBFactory::Backend* used(nullptr);
    auto db(DBFactory::connect("", {}, used, fakes.creator()));
    BOOST_TEST(used->name == "mysql");
    BOOST_TEST(fakes.tried == Names{"mysql"}, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(SkipsMissingModule) {
    Fakes fakes{.servers = {{"mysql", Server::MISSING}, {"postgresql", Server::ACCEPTING}}};
    const DBFactory::Backend* used(nullptr);
    auto db(DBFactory::connect("auto", {}, used, fakes.creator()));
    BOOST_TEST(used->name == "postgresql");
}

// A server rejecting the login is reported, even if another one isn't found afterwards
BOOST_AUTO_TEST_CASE(ReportsRefusal) {
    Fakes fakes{.servers = {{"mysql", Server::REFUSING}, {"postgresql", Server::NONE}}};
    const DBFactory::Backend* used(nullptr);
    try {
        auto db(DBFactory::connect("auto", {}, used, fakes.creator()));
        BOOST_FAIL("Connecting should fail");
    }
    catch (DBUnreachable&) {
        BOOST_FAIL("The refusal should be reported");
    }
    catch (std::runtime_error& err) {
        BOOST_TEST(std::string(err.what()) == "MySQL/MariaDB: Access denied");
    }
    BOOST_TEST(fakes.tried == (Names{"mysql", "postgresql"}), boost::test_tools::per_element());
    BOOST_TEST(!used);
}

// After a server answered at an explicit port or socket, the other backends aren't tried there
BOOST_AUTO_TEST_CASE(RefusalAtExplicitAddress) {
    for (const ConnectParams& params : {ConnectParams{.port = 5432}, ConnectParams{.host = "/run/postgresql/.s.PGSQL.5432"}}) {
        Fakes fakes{.servers = {{"mysql", Server::ACCEPTING}, {"postgresql", Server::REFUSING}}};
        const DBFactory::Backend* used(nullptr);
        BOOST_CHECK_THROW(auto db(DBFactory::connect("auto", params, used, fakes.creator())), std::runtime_error);
        BOOST_TEST(fakes.tried == Names{"postgresql"}, boost::test_tools::per_element());
    }
}

BOOST_AUTO_TEST_CASE(NoServer) {
    Fakes fakes{.servers = {{"mysql", Server::NONE}, {"postgresql", Server::MISSING}}};
    const DBFactory::Backend* used(nullptr);
    BOOST_CHECK_THROW(auto db(DBFactory::connect("auto", {}, used, fakes.creator())), DBUnreachable);
}

BOOST_AUTO_TEST_CASE(NoBackend) {
    Fakes fakes{.servers = {{"mysql", Server::MISSING}, {"postgresql", Server::MISSING}}};
    const DBFactory::Backend* used(nullptr);
    try {
        auto db(DBFactory::connect("auto", {}, used, fakes.creator()));
        BOOST_FAIL("Connecting should fail");
    }
    catch (DBUnreachable&) {
        BOOST_FAIL("Missing modules aren't missing servers");
    }
    catch (std::runtime_error&) {
    }
}

// An explicitly chosen backend reports its errors unchanged
BOOST_AUTO_TEST_CASE(ExplicitBackend) {
    Fakes fakes{.servers = {{"mysql", Server::ACCEPTING}, {"postgresql", Server::NONE}}};
    const DBFactory::Backend* used(nullptr);
    BOOST_CHECK_THROW(auto db(DBFactory::connect("postgresql", {}, used, fakes.creator())), DBUnreachable);
    BOOST_TEST(fakes.tried == Names{"postgresql"}, boost::test_tools::per_element());
    BOOST_CHECK_THROW(auto db(DBFactory::connect("oracle", {}, used, fakes.creator())), std::invalid_argument);
}

// The real modules (from the build directory) can be loaded
BOOST_AUTO_TEST_CASE(BuiltModules) {
    for (const auto& backend : DBFactory::backends) {
        BOOST_TEST_CONTEXT(backend.name) {
            if (std::string_view(BUILT_BACKENDS).contains(backend.name)) {
                auto db(DBFactory::create(backend));
                BOOST_REQUIRE(db);
                BOOST_TEST(!db->connected());
            }
        }
    }
}

// The real modules: Loading a not existing one fails
BOOST_AUTO_TEST_CASE(MissingModule) {
    const std::string old(std::getenv("CDMGR_MODULEDIR") ? std::getenv("CDMGR_MODULEDIR") : "");
    setenv("CDMGR_MODULEDIR", "/nonexistent", 1);
    BOOST_CHECK_THROW(auto db(DBFactory::create(DBFactory::backends[0])), std::runtime_error);
    setenv("CDMGR_MODULEDIR", old.c_str(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
