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

#include <locale>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "DB.h"

#define BOOST_TEST_MODULE Database
#include "UnitTest.h"

namespace {

/// Database recording the executed queries and returning prepared results
class FakeDatabase : public Database {
  public:
    void connect(const char*, const char*, const char*) override { isConnected = true; }
    void close() override { isConnected = false; }
    [[nodiscard]] bool connected() const override { return isConnected; }

    long getIDOfInsert() override { return 42; }

    [[nodiscard]] std::string escapeDBValue(const std::string& value) const override {
        std::string escaped;
        for (char ch : value) {
            if (ch == '\'')
                escaped += '\'';
            escaped += ch;
        }
        return escaped;
    }
    [[nodiscard]] std::string quoteBlob(const std::string& value) const override { return "BLOB(" + value + ')'; }

    std::vector<std::string> queries;
    std::vector<Row> result;

  protected:
    void query(const char* query, std::vector<Row>& rows) override {
        queries.emplace_back(query);
        rows = std::move(result);
        result.clear();
    }

  private:
    bool isConnected{true};
};

/// Number formatting with thousands separators (like in many locales)
struct GroupingPunct : std::numpunct<char> {
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

/// Fixture setting a global locale with thousands separators
struct GroupingLocale {
    GroupingLocale() : old(std::locale::global(std::locale(std::locale::classic(), new GroupingPunct))) {}
    ~GroupingLocale() { std::locale::global(old); }

    std::locale old;
};

} // namespace

BOOST_AUTO_TEST_SUITE(Statements)

BOOST_AUTO_TEST_CASE(Insert) {
    FakeDatabase db;
    db.insert("Films", Database::Values()("name", db.quote("Alien"))("year", 1979)("genre", 2U));
    BOOST_REQUIRE(db.queries.size() == 1U);
    BOOST_TEST(db.queries[0] == "INSERT INTO Films (name, year, genre) VALUES ('Alien', 1979, 2)");
}

BOOST_AUTO_TEST_CASE(InsertSingleValue) {
    FakeDatabase db;
    db.insert("Words", Database::Values()("word", db.quote("The")));
    BOOST_REQUIRE(db.queries.size() == 1U);
    BOOST_TEST(db.queries[0] == "INSERT INTO Words (word) VALUES ('The')");
}

BOOST_AUTO_TEST_CASE(Update) {
    FakeDatabase db;
    db.update("Films", Database::Values()("name", db.quote("Alien"))("year", 1979), "id=7");
    BOOST_REQUIRE(db.queries.size() == 1U);
    BOOST_TEST(db.queries[0] == "UPDATE Films SET name='Alien', year=1979 WHERE id=7");
}

BOOST_AUTO_TEST_CASE(Execute) {
    FakeDatabase db;
    db.execute("SELECT 1");
    db.execute(std::string("SELECT 2"));
    BOOST_TEST(db.queries == (std::vector<std::string> {"SELECT 1", "SELECT 2"}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(Quote) {
    FakeDatabase db;
    BOOST_TEST(db.quote("Alien") == "'Alien'");
    BOOST_TEST(db.quote("Schindler's List") == "'Schindler''s List'");
    BOOST_TEST(db.quote("") == "''");
}

// SQL needs numbers without thousands separators, whatever the locale is
BOOST_FIXTURE_TEST_CASE(NumbersIgnoreTheLocale, GroupingLocale) {
    std::ostringstream check;
    check << 2026;
    BOOST_REQUIRE(check.str() == "2.026");

    FakeDatabase db;
    db.insert("Films", Database::Values()("year", 2026)("id", 1234567UL)("size", -1000L));
    BOOST_REQUIRE(db.queries.size() == 1U);
    BOOST_TEST(db.queries[0] == "INSERT INTO Films (year, id, size) VALUES (2026, 1234567, -1000)");
}

BOOST_AUTO_TEST_CASE(Values) {
    const Database::Values empty;
    BOOST_TEST(empty.empty());

    const auto values(Database::Values()("a", 1)("b", std::string("text"))("c", 'x'));
    BOOST_TEST(!values.empty());
    using Entries = std::vector<std::pair<std::string, std::string>>;
    BOOST_TEST((Entries(values.begin(), values.end()) == Entries {{"a", "1"}, {"b", "text"}, {"c", "x"}}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Results)

BOOST_AUTO_TEST_CASE(IterateRows) {
    FakeDatabase db;
    db.result = {{"1", "Alien", "1979"}, {"2", "Brazil", "1985"}};
    db.execute("SELECT id, name, year FROM Films");

    BOOST_TEST(db.resultSize() == 2U);
    BOOST_REQUIRE(db.hasData());
    BOOST_TEST(db.getResultColumnAsUInt(0) == 1U);
    BOOST_TEST(db.getResultColumnAsString(1) == "Alien");
    BOOST_TEST(db.getResultColumnAsInt(2) == 1979);

    db.getNextResultRow();
    BOOST_REQUIRE(db.hasData());
    BOOST_TEST(db.getResultColumnAsUInt(0) == 2U);
    BOOST_TEST(db.getResultColumnAsString(1) == "Brazil");

    db.getNextResultRow();
    BOOST_TEST(!db.hasData());
}

BOOST_AUTO_TEST_CASE(EmptyResult) {
    FakeDatabase db;
    db.execute("SELECT * FROM Films WHERE 0=1");
    BOOST_TEST(db.resultSize() == 0U);
    BOOST_TEST(!db.hasData());
}

// Executing a new query discards the old result and restarts with the first row
BOOST_AUTO_TEST_CASE(NewQueryResetsResult) {
    FakeDatabase db;
    db.result = {{"1"}, {"2"}};
    db.execute("SELECT id FROM Films");
    db.getNextResultRow();
    db.getNextResultRow();
    BOOST_TEST(!db.hasData());

    db.result = {{"3"}};
    db.execute("SELECT id FROM Records");
    BOOST_TEST(db.resultSize() == 1U);
    BOOST_REQUIRE(db.hasData());
    BOOST_TEST(db.getResultColumnAsUInt(0) == 3U);
}

BOOST_AUTO_TEST_CASE(Conversions) {
    FakeDatabase db;
    db.result = {{"-5", "", "abc", "12abc", std::string("bin\0ary", 7)}};
    db.execute("SELECT");
    BOOST_TEST(db.getResultColumnAsInt(0) == -5);
    BOOST_TEST(db.getResultColumnAsInt(1) == 0);
    BOOST_TEST(db.getResultColumnAsUInt(1) == 0U);
    BOOST_TEST(db.getResultColumnAsInt(2) == 0);
    BOOST_TEST(db.getResultColumnAsUInt(3) == 12U);
    BOOST_TEST(db.getResultColumnAsBlob(4) == std::string("bin\0ary", 7));
    BOOST_TEST(db.getResultColumnAsBlob(4).size() == 7U);
}

BOOST_AUTO_TEST_SUITE_END()
