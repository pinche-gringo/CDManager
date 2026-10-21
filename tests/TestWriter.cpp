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

#include <memory>
#include <sstream>
#include <string>

#include "Writer.h"

#define BOOST_TEST_MODULE Writer
#include "UnitTest.h"

namespace {

/// Genres of both films and records
struct AllGenres {
    Genres records;
    Genres films;

    AllGenres() { Genres::loadFromFile(TESTDATADIR "Genres.dat", records, films, ""); }
};

HCelebrity makeCelebrity(const char* name) {
    auto celeb(std::make_shared<Celebrity>());
    celeb->setName(name);
    return celeb;
}

HFilm makeFilm(const char* name, int year, unsigned int genre, int type, const char* lang, const char* titles) {
    auto film(std::make_shared<Film>());
    film->setName(name);
    if (year)
        film->setYear(year);
    film->setGenre(genre);
    film->setType(type);
    film->setLanguage(lang);
    film->setTitles(titles);
    return film;
}

HRecord makeRecord(const char* name, int year, unsigned int genre) {
    auto record(std::make_shared<Record>());
    record->setName(name);
    if (year)
        record->setYear(year);
    record->setGenre(genre);
    return record;
}

const std::string DIRECTOR_ROW("<tr><td>&nbsp;</td></tr>\n\n<tr><td colspan=\"5\" class=\"owner\">");
const std::string INTERPRET_ROW("<tr><td>&nbsp;</td></tr>\n\n<tr><td colspan=\"3\" class=\"owner\">");

} // namespace

BOOST_FIXTURE_TEST_SUITE(Films, AllGenres)

BOOST_AUTO_TEST_CASE(WriteFilm) {
    const std::string format("%n|%y|%g|%d|%t|%l"); // The writer keeps a reference to the format
    FilmWriter writer(format, films);
    auto film(makeFilm("Alien", 1979, 2, 1, "en", ""));
    film->setDescription("In space no one can hear you scream");

    std::ostringstream out;
    writer.writeFilm(film, makeCelebrity("Ridley Scott"), out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\" title=\"In space no one can hear you scream\"><td>Alien</td><td>1979</td>"
                  "<td>Drama</td><td>Ridley Scott</td><td>1 DVD</td><td><img src=\"images/en.png\" alt=\"en \"></td></tr>\n");
}

BOOST_AUTO_TEST_CASE(SpecialCharacters) {
    const std::string format("%n|%d");
    FilmWriter writer(format, films);
    auto film(makeFilm("Tom & Jerry <The Movie>", 1992, 1, 0, "", ""));
    film->setDescription("\"Cat & mouse\"");

    std::ostringstream out;
    writer.writeFilm(film, makeCelebrity("Phil <Roman>"), out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\" title=\"&quot;Cat &amp; mouse&quot;\"><td>Tom &amp; Jerry &lt;The Movie&gt;</td>"
                  "<td>Phil &lt;Roman&gt;</td></tr>\n");
}

BOOST_AUTO_TEST_CASE(Languages) {
    const std::string format("%l");
    FilmWriter writer(format, films);

    std::ostringstream out;
    writer.writeFilm(makeFilm("Alien", 1979, 2, 1, "en,de", "es,,fr,"), makeCelebrity("Ridley Scott"), out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\" title=\"\"><td><img src=\"images/en.png\" alt=\"en \">"
                  "<img src=\"images/de.png\" alt=\"de \"> &ndash; <img src=\"images/es.png\" alt=\"es \">"
                  "<img src=\"images/fr.png\" alt=\"fr \"></td></tr>\n");
}

// Empty columns contain a blank, unknown control characters are written as is
BOOST_AUTO_TEST_CASE(EmptyAndUnknownColumns) {
    const std::string format("%y|%l|%x|text");
    FilmWriter writer(format, films);

    std::ostringstream out;
    writer.writeFilm(makeFilm("Alien", 0, 2, 1, "", ""), makeCelebrity("Ridley Scott"), out);
    BOOST_TEST(out.str() == "<tr class=\"odd\" title=\"\"><td> </td><td> </td><td>x</td><td>text</td></tr>\n");
}

// The lines alternate between odd and even; a director starts again with odd
BOOST_AUTO_TEST_CASE(AlternatingLines) {
    const std::string format("%n");
    FilmWriter writer(format, films);
    const auto director(makeCelebrity("Ridley Scott"));

    std::ostringstream out;
    writer.writeFilm(makeFilm("Alien", 1979, 2, 1, "", ""), director, out);
    writer.writeFilm(makeFilm("Blade Runner", 1982, 2, 1, "", ""), director, out);
    writer.writeFilm(makeFilm("Gladiator", 2000, 2, 1, "", ""), director, out);
    writer.writeDirector(makeCelebrity("Terry Gilliam"), out);
    writer.writeFilm(makeFilm("Brazil", 1985, 1, 1, "", ""), director, out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\" title=\"\"><td>Alien</td></tr>\n"
                  "<tr class=\"even\" title=\"\"><td>Blade Runner</td></tr>\n"
                  "<tr class=\"odd\" title=\"\"><td>Gladiator</td></tr>\n"
                  + DIRECTOR_ROW + "Terry Gilliam</td></tr>\n"
                  "<tr class=\"odd\" title=\"\"><td>Brazil</td></tr>\n");
}

BOOST_AUTO_TEST_CASE(WriteDirector) {
    const std::string format("%n");
    FilmWriter writer(format, films);

    std::ostringstream out;
    writer.writeDirector(makeCelebrity("Ridley Scott"), out);
    BOOST_TEST(out.str() == DIRECTOR_ROW + "Ridley Scott</td></tr>\n");
}

BOOST_AUTO_TEST_CASE(WriteDirectorSpecialCharacters) {
    const std::string format("%n");
    FilmWriter writer(format, films);

    std::ostringstream out;
    writer.writeDirector(makeCelebrity("Simon & Garfunkel"), out);
    BOOST_TEST(out.str() == DIRECTOR_ROW + "Simon &amp; Garfunkel</td></tr>\n");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(Records, AllGenres)

BOOST_AUTO_TEST_CASE(WriteRecord) {
    const std::string format("%n|%y|%g|%d");
    RecordWriter writer(format, records);

    std::ostringstream out;
    writer.writeRecord(makeRecord("Abbey Road", 1969, 1), makeCelebrity("The Beatles"), out);
    writer.writeRecord(makeRecord("Bridge over Troubled Water", 1970, 3), makeCelebrity("Simon & Garfunkel"), out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\"><td>Abbey Road</td><td>1969</td><td>Rock</td><td>The Beatles</td></tr>\n"
                  "<tr class=\"even\"><td>Bridge over Troubled Water</td><td>1970</td><td>Jazz</td>"
                  "<td>Simon &amp; Garfunkel</td></tr>\n");
}

BOOST_AUTO_TEST_CASE(EmptyAndUnknownColumns) {
    const std::string format("%y|%x");
    RecordWriter writer(format, records);

    std::ostringstream out;
    writer.writeRecord(makeRecord("Abbey Road", 0, 1), makeCelebrity("The Beatles"), out);
    BOOST_TEST(out.str() == "<tr class=\"odd\"><td> </td><td>x</td></tr>\n");
}

BOOST_AUTO_TEST_CASE(WriteInterpret) {
    const std::string format("%n");
    RecordWriter writer(format, records);

    std::ostringstream out;
    writer.writeRecord(makeRecord("Abbey Road", 1969, 1), makeCelebrity("The Beatles"), out);
    writer.writeInterpret(makeCelebrity("Simon & Garfunkel"), out);
    writer.writeRecord(makeRecord("Bookends", 1968, 1), makeCelebrity("Simon & Garfunkel"), out);
    BOOST_TEST(out.str()
               == "<tr class=\"odd\"><td>Abbey Road</td></tr>\n" + INTERPRET_ROW
                      + "Simon &amp; Garfunkel</td></tr>\n<tr class=\"odd\"><td>Bookends</td></tr>\n");
}

BOOST_AUTO_TEST_SUITE_END()
