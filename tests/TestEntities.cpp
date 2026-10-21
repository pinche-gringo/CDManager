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

#include <algorithm>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <YGP/ADate.h>

#include "Celebrity.h"
#include "Film.h"
#include "Record.h"
#include "WordsFixture.h"

#define BOOST_TEST_MODULE Entities
#include "UnitTest.h"

namespace {

/// Articles and names used by the tests
struct EntityWords : WordsFixture {
    EntityWords() : WordsFixture({"The", "A", "Die"}, {"Mr", "Dr", "Sir"}) {}
};

HCelebrity makeCelebrity(const char* name, unsigned long id = 0) {
    auto celeb(std::make_shared<Celebrity>());
    celeb->setName(name);
    celeb->setId(id);
    return celeb;
}

HFilm makeFilm(const char* name, int year = 0, unsigned int genre = 0, int type = 0) {
    auto film(std::make_shared<Film>());
    film->setName(name);
    if (year)
        film->setYear(year);
    film->setGenre(genre);
    film->setType(type);
    return film;
}

HRecord makeRecord(const char* name, int year = 0, unsigned int genre = 0) {
    auto record(std::make_shared<Record>());
    record->setName(name);
    if (year)
        record->setYear(year);
    record->setGenre(genre);
    return record;
}

/// Sorts the passed entities with the passed function and returns their names
template <typename T, typename Comp> std::vector<std::string> sortedNames(std::vector<T> entities, Comp comp) {
    std::ranges::stable_sort(entities, comp);
    std::vector<std::string> names;
    for (const auto& entity : entities)
        names.emplace_back(entity->getName());
    return names;
}

/// Returns the current year
int thisYear() { return YGP::ADate::today().getYear(); }

} // namespace

BOOST_AUTO_TEST_SUITE(Celebrities)

BOOST_FIXTURE_TEST_CASE(RemoveIgnored, EntityWords) {
    BOOST_TEST(Celebrity::removeIgnored("The Rolling Stones") == "Rolling Stones");
    BOOST_TEST(Celebrity::removeIgnored("Mr Bean") == "Bean");
    BOOST_TEST(Celebrity::removeIgnored("Sir Dr J. Smith") == "Smith");
    BOOST_TEST(Celebrity::removeIgnored("Madonna") == "Madonna");
}

BOOST_FIXTURE_TEST_CASE(SortByName, EntityWords) {
    const std::vector<HCelebrity> celebs {makeCelebrity("The Who"), makeCelebrity("ABBA"), makeCelebrity("Mr Bean"),
                                          makeCelebrity("The Beatles"), makeCelebrity("Sir Elton John")};
    BOOST_TEST(sortedNames(celebs, Celebrity::compByName)
                   == (std::vector<std::string> {"ABBA", "Mr Bean", "The Beatles", "Sir Elton John", "The Who"}),
               boost::test_tools::per_element());
}

// Equal names (after removing the ignored parts) are sorted by their full name
BOOST_FIXTURE_TEST_CASE(SortByNameEqualNames, EntityWords) {
    const auto a(makeCelebrity("The Doors")), b(makeCelebrity("Doors"));
    BOOST_TEST(Celebrity::compByName(b, a));
    BOOST_TEST(!Celebrity::compByName(a, b));
    BOOST_TEST(!Celebrity::compByName(a, a));
}

BOOST_AUTO_TEST_CASE(SortById) {
    const auto a(makeCelebrity("B", 1)), b(makeCelebrity("A", 2));
    BOOST_TEST(Celebrity::compById(a, b));
    BOOST_TEST(!Celebrity::compById(b, a));
    BOOST_TEST(!Celebrity::compById(a, a));
}

BOOST_AUTO_TEST_CASE(CopyAndAssign) {
    Celebrity celeb;
    celeb.setId(7);
    celeb.setName("Prince");
    celeb.setLifespan("1958 - 2016");

    const Celebrity copy(celeb);
    BOOST_TEST(copy.getId() == 7UL);
    BOOST_TEST(copy.getName() == "Prince");
    BOOST_TEST(copy.getLifespan() == "1958 - 2016");

    // Assigning keeps an already set ID
    Celebrity other;
    other.setId(3);
    other = celeb;
    BOOST_TEST(other.getId() == 3UL);
    BOOST_TEST(other.getName() == "Prince");
    BOOST_TEST(other.getLifespan() == "1958 - 2016");

    Celebrity unset;
    unset = celeb;
    BOOST_TEST(unset.getId() == 7UL);
}

// The attributes (used for the database) are registered by reflection
BOOST_AUTO_TEST_CASE(Attributes) {
    Celebrity celeb;
    celeb.setName("Prince");
    celeb.setBorn(std::string("1958"));

    BOOST_REQUIRE(celeb.findAttribute("Name"));
    BOOST_TEST(celeb.findAttribute("Name")->getValue() == "Prince");
    BOOST_REQUIRE(celeb.findAttribute("Born"));
    BOOST_TEST(celeb.findAttribute("Born")->getValue() == "1958");
    BOOST_REQUIRE(celeb.findAttribute("Died"));
    BOOST_TEST(celeb.findAttribute("Died")->getValue() == "");
    BOOST_TEST(!celeb.findAttribute("id"));

    BOOST_TEST(celeb.findAttribute("Died")->assignFromString("2016"));
    BOOST_TEST(celeb.getDied() == YGP::AYear(2016));

    // A copy must have its own attributes
    const Celebrity copy(celeb);
    celeb.setName("Other");
    BOOST_TEST(copy.findAttribute("Name")->getValue() == "Prince");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Lifespan)

BOOST_AUTO_TEST_CASE(BornAndDied) {
    Celebrity celeb;
    celeb.setLifespan("1920 - 1990");
    BOOST_TEST(celeb.getBorn() == YGP::AYear(1920));
    BOOST_TEST(celeb.getDied() == YGP::AYear(1990));
    BOOST_TEST(celeb.getLifespan() == "1920 - 1990");
}

BOOST_AUTO_TEST_CASE(OnlyBorn) {
    Celebrity celeb;
    celeb.setLifespan("1958");
    BOOST_TEST(celeb.getBorn() == YGP::AYear(1958));
    BOOST_TEST(!celeb.getDied().isDefined());
    BOOST_TEST(celeb.getLifespan() == "1958");
}

BOOST_AUTO_TEST_CASE(OnlyDied) {
    Celebrity celeb;
    celeb.setLifespan("- 1990");
    BOOST_TEST(!celeb.getBorn().isDefined());
    BOOST_TEST(celeb.getDied() == YGP::AYear(1990));
    BOOST_TEST(celeb.getLifespan() == "- 1990");
}

BOOST_AUTO_TEST_CASE(Empty) {
    Celebrity celeb;
    celeb.setLifespan("1920 - 1990");
    celeb.setLifespan("");
    BOOST_TEST(!celeb.getBorn().isDefined());
    BOOST_TEST(!celeb.getDied().isDefined());
    BOOST_TEST(celeb.getLifespan() == "");
}

BOOST_AUTO_TEST_CASE(Boundaries) {
    Celebrity celeb;
    celeb.setLifespan("1850");
    BOOST_TEST(celeb.getBorn() == YGP::AYear(1850));

    const std::string now(std::format("{}", thisYear()));
    celeb.setLifespan(now + " - " + now);
    BOOST_TEST(celeb.getLifespan() == now + " - " + now);
}

BOOST_AUTO_TEST_CASE(Invalid) {
    Celebrity celeb;
    BOOST_CHECK_THROW(celeb.setLifespan("1849"), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan(std::format("{}", thisYear() + 1)), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan("1990 - 1920"), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan(std::format("1990 - {}", thisYear() + 1)), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan("1920-1990"), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan("born 1920"), std::invalid_argument);
    BOOST_CHECK_THROW(celeb.setLifespan("1920 - died"), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Films)

BOOST_AUTO_TEST_CASE(NamesPerLanguage) {
    Film::currLang = "";
    Film film;
    film.setName("Downfall");
    BOOST_TEST(film.getName() == "Downfall");

    // The first name is the default one; further names are stored for the current language
    Film::currLang = "de";
    film.setName("Der Untergang");
    BOOST_TEST(film.getName() == "Der Untergang");
    BOOST_TEST(film.getName("de") == "Der Untergang");
    BOOST_TEST(film.getName("") == "Downfall");
    BOOST_TEST(film.getName("es") == "Downfall");
    BOOST_TEST(film.getNames().size() == 2U);

    film.setName("El hundimiento", "es");
    BOOST_TEST(film.getName("es") == "El hundimiento");
    BOOST_TEST(film.getName() == "Der Untergang");

    Film::currLang = "fr";
    BOOST_TEST(film.getName() == "Downfall");
    Film::currLang = "";
}

BOOST_AUTO_TEST_CASE(FirstNameIsDefault) {
    Film::currLang = "de";
    Film film;
    film.setName("Der Untergang");
    BOOST_TEST(film.getName("") == "Der Untergang");
    BOOST_TEST(film.getNames().size() == 1U);
    Film::currLang = "";
}

BOOST_AUTO_TEST_CASE(RemoveName) {
    Film film;
    film.setName("Downfall");
    film.setName("Der Untergang", "de");
    film.setName("", "de");
    BOOST_TEST(film.getNames().size() == 1U);
    BOOST_TEST(film.getName("de") == "Downfall");
}

// Removing a name, which doesn't exist, must not add an empty one
BOOST_AUTO_TEST_CASE(RemoveMissingName) {
    Film film;
    film.setName("Downfall");
    film.setName("", "fr");
    BOOST_TEST(film.getNames().size() == 1U);
    BOOST_TEST(film.getName("fr") == "Downfall");
}

// The name-attribute (used for the database) handles the name of the current language
BOOST_AUTO_TEST_CASE(NameAttribute) {
    Film::currLang = "";
    Film film;
    film.setName("Downfall");
    film.setName("Der Untergang", "de");

    const YGP::IAttribute* name(film.findAttribute("Name"));
    BOOST_REQUIRE(name);
    BOOST_TEST(name->getValue() == "Downfall");
    Film::currLang = "de";
    BOOST_TEST(name->getValue() == "Der Untergang");
    Film::currLang = "es";
    BOOST_TEST(name->getValue() == "Downfall");

    BOOST_TEST(name->assignFromString("El hundimiento"));
    BOOST_TEST(film.getName("es") == "El hundimiento");
    BOOST_TEST(film.getName("") == "Downfall");
    Film::currLang = "";
}

BOOST_AUTO_TEST_CASE(OtherAttributes) {
    Film film;
    film.setName("Downfall");
    film.setYear(2004);
    film.setGenre(6);
    film.setType(1);
    film.setLanguage("de");
    film.setTitles("en,es");
    film.setDescription("Traudl Junge, the final secretary for Adolf Hitler, ...");

    for (const auto& [attrib, value] : std::vector<std::pair<const char*, const char*>> {
             {"Made", "2004"}, {"Genre", "6"}, {"Media", "1"}, {"Lang", "de"}, {"Subtitles", "en,es"}}) {
        BOOST_TEST_CONTEXT(attrib) {
            BOOST_REQUIRE(film.findAttribute(attrib));
            BOOST_TEST(film.findAttribute(attrib)->getValue() == value);
        }
    }
    BOOST_REQUIRE(film.findAttribute("Description"));
    BOOST_TEST(film.findAttribute("Description")->getValue() == film.getDescription());
}

BOOST_AUTO_TEST_CASE(CopyAndAssign) {
    Film film;
    film.setId(5);
    film.setName("Downfall");
    film.setName("Der Untergang", "de");
    film.setYear(2004);
    film.setImage("poster");

    Film copy(film);
    BOOST_TEST(copy.getId() == 5UL);
    BOOST_TEST(copy.getName("de") == "Der Untergang");
    BOOST_TEST(copy.getYear() == YGP::AYear(2004));
    BOOST_TEST(copy.getImage() == "poster");

    // Assigning keeps an already set ID
    Film other;
    other.setId(3);
    other = film;
    BOOST_TEST(other.getId() == 3UL);
    BOOST_TEST(other.getName("") == "Downfall");
}

BOOST_AUTO_TEST_CASE(Media) {
    Film film;
    film.setType(2);
    BOOST_TEST(film.getType() == 2);
    film.setType(std::string("1 CD"));
    BOOST_TEST(film.getType() == 3);

    BOOST_CHECK_THROW(film.setType(99), std::out_of_range);
    BOOST_CHECK_THROW(film.setType(std::string("Betamax")), std::out_of_range);
    BOOST_TEST(film.getType() == 3);
}

BOOST_FIXTURE_TEST_CASE(SortByName, EntityWords) {
    const std::vector<HFilm> films {makeFilm("The Thing", 2011), makeFilm("Alien", 1979), makeFilm("The Thing", 1982),
                                    makeFilm("A Beautiful Mind", 2001), makeFilm("Zardoz", 1974)};
    BOOST_TEST(sortedNames(films, Film::compByName)
                   == (std::vector<std::string> {"Alien", "A Beautiful Mind", "The Thing", "The Thing", "Zardoz"}),
               boost::test_tools::per_element());

    // Equal names are sorted by year
    BOOST_TEST(Film::compByName(films[2], films[0]));
    BOOST_TEST(!Film::compByName(films[0], films[2]));
}

BOOST_FIXTURE_TEST_CASE(SortByYear, EntityWords) {
    const std::vector<HFilm> films {makeFilm("Zardoz", 1974), makeFilm("The Thing", 1982), makeFilm("Alien", 1979),
                                    makeFilm("Blade Runner", 1982)};
    BOOST_TEST(sortedNames(films, Film::compByYear)
                   == (std::vector<std::string> {"Zardoz", "Alien", "Blade Runner", "The Thing"}),
               boost::test_tools::per_element());
}

// Films without year come first (sorted by their names)
BOOST_FIXTURE_TEST_CASE(SortByYearUndefined, EntityWords) {
    const std::vector<HFilm> films {makeFilm("Zardoz", 1974), makeFilm("The Thing"), makeFilm("Alien")};
    BOOST_TEST(sortedNames(films, Film::compByYear) == (std::vector<std::string> {"Alien", "The Thing", "Zardoz"}),
               boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(SortByGenre, EntityWords) {
    const std::vector<HFilm> films {makeFilm("Zardoz", 1974, 2), makeFilm("The Thing", 1982, 1), makeFilm("Alien", 1979, 2),
                                    makeFilm("Brazil", 1985, 0)};
    BOOST_TEST(sortedNames(films, Film::compByGenre)
                   == (std::vector<std::string> {"Brazil", "The Thing", "Alien", "Zardoz"}),
               boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(SortByMedia, EntityWords) {
    const std::vector<HFilm> films {makeFilm("Zardoz", 1974, 0, 1), makeFilm("The Thing", 1982, 0, 2),
                                    makeFilm("Alien", 1979, 0, 1), makeFilm("Brazil", 1985, 0, 0)};
    BOOST_TEST(sortedNames(films, Film::compByMedia)
                   == (std::vector<std::string> {"Brazil", "Alien", "Zardoz", "The Thing"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Records)

BOOST_FIXTURE_TEST_CASE(SortByName, EntityWords) {
    const std::vector<HRecord> records {makeRecord("The Wall"), makeRecord("Abbey Road"), makeRecord("A Night at the Opera"),
                                        makeRecord("Die Mensch-Maschine")};
    BOOST_TEST(sortedNames(records, Record::compByName)
                   == (std::vector<std::string> {"Abbey Road", "Die Mensch-Maschine", "A Night at the Opera", "The Wall"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(SortByYear) {
    const std::vector<HRecord> records {makeRecord("The Wall", 1979), makeRecord("Abbey Road", 1969),
                                        makeRecord("Thriller", 1982)};
    BOOST_TEST(sortedNames(records, Record::compByYear)
                   == (std::vector<std::string> {"Abbey Road", "The Wall", "Thriller"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(SortByGenre) {
    const std::vector<HRecord> records {makeRecord("The Wall", 1979, 8), makeRecord("Abbey Road", 1969, 2),
                                        makeRecord("Thriller", 1982, 5)};
    BOOST_TEST(sortedNames(records, Record::compByGenre)
                   == (std::vector<std::string> {"Abbey Road", "Thriller", "The Wall"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(SongsLoaded) {
    Record record;
    BOOST_TEST(record.needsLoading());
    record.setSongsLoaded();
    BOOST_TEST(!record.needsLoading());

    const Record copy(record);
    BOOST_TEST(!copy.needsLoading());
}

BOOST_AUTO_TEST_CASE(Attributes) {
    Record record;
    record.setName("Abbey Road");
    record.setYear(1969);
    record.setGenre(2);
    BOOST_REQUIRE(record.findAttribute("Name"));
    BOOST_TEST(record.findAttribute("Name")->getValue() == "Abbey Road");
    BOOST_REQUIRE(record.findAttribute("Made"));
    BOOST_TEST(record.findAttribute("Made")->getValue() == "1969");
    BOOST_REQUIRE(record.findAttribute("Genre"));
    BOOST_TEST(record.findAttribute("Genre")->getValue() == "2");
}

BOOST_AUTO_TEST_SUITE_END()
