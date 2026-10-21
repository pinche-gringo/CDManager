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

#include <stdexcept>
#include <string>
#include <vector>

#include <glibmm/ustring.h>

#include "Genres.h"

#define BOOST_TEST_MODULE Genres
#include "UnitTest.h"

namespace {

const std::string FILE(TESTDATADIR "Genres.dat");

/// Genres of both films and records
struct AllGenres {
    Genres records;
    Genres films;

    explicit AllGenres(const char* languages, const std::string& file = FILE) {
        Genres::loadFromFile(file.c_str(), records, films, languages);
    }
};

/// Returns all passed genres
std::vector<Glib::ustring> genres(const Genres& genres) {
    std::vector<Glib::ustring> values;
    for (unsigned int i(0); i < genres.size(); ++i)
        values.push_back(genres.getGenre(i));
    return values;
}

} // namespace

BOOST_AUTO_TEST_CASE(Default) {
    const AllGenres all("");
    BOOST_TEST(genres(all.films) == (std::vector<Glib::ustring> {"Undefined", "Comedy", "Drama"}),
               boost::test_tools::per_element());
    BOOST_TEST(genres(all.records) == (std::vector<Glib::ustring> {"Undefined", "Rock", "Classic", "Jazz"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(Translated) {
    const AllGenres all("de");
    BOOST_TEST(genres(all.films) == (std::vector<Glib::ustring> {"Undefiniert", "Komödie", "Drama"}),
               boost::test_tools::per_element());
    BOOST_TEST(genres(all.records) == (std::vector<Glib::ustring> {"Undefiniert", "Rock", "Klassik", "Jazz"}),
               boost::test_tools::per_element());
}

// Languages are passed like the LANGUAGE environment variable: Colon-separated
// and possibly with country, encoding or modifier
BOOST_AUTO_TEST_CASE(LanguageSelection) {
    BOOST_TEST(AllGenres("de_AT.UTF-8").films.getGenre(1) == "Komödie");
    BOOST_TEST(AllGenres("de_AT").films.getGenre(1) == "Komödie");
    BOOST_TEST(AllGenres("es_MX").films.getGenre(1) == "Comedia");
    BOOST_TEST(AllGenres("es").films.getGenre(1) == "Comedy");
    BOOST_TEST(AllGenres("fr:de").films.getGenre(1) == "Komödie");
    BOOST_TEST(AllGenres("::de").films.getGenre(1) == "Komödie");
    BOOST_TEST(AllGenres("de:es_MX").films.getGenre(1) == "Komödie");
    BOOST_TEST(AllGenres("fr:it").films.getGenre(1) == "Comedy");
}

BOOST_AUTO_TEST_CASE(GetId) {
    const AllGenres all("de");
    BOOST_TEST(all.films.getId("Undefiniert") == 0);
    BOOST_TEST(all.films.getId("Drama") == 2);
    BOOST_TEST(all.records.getId("Jazz") == 3);
    BOOST_TEST(all.records.getId("Comedy") == -1);
    BOOST_TEST(all.records.getId("") == -1);
}

BOOST_AUTO_TEST_CASE(MissingFile) {
    Genres records, films;
    BOOST_CHECK_THROW(Genres::loadFromFile(TESTDATADIR "Missing.dat", records, films, "de"), std::exception);
}

// The shipped translations must have the same genres (in the same order) as the default
BOOST_AUTO_TEST_CASE(ShippedTranslations) {
    const std::string file(SRCDATADIR "Genres.dat");
    const AllGenres all("", file);
    BOOST_TEST(all.films.size() > 1U);
    BOOST_TEST(all.records.size() > 1U);

    for (const char* lang : {"de", "es"}) {
        BOOST_TEST_CONTEXT("Language " << lang) {
            const AllGenres translated(lang, file);
            BOOST_TEST(translated.films.size() == all.films.size());
            BOOST_TEST(translated.records.size() == all.records.size());
            BOOST_TEST(translated.films.getGenre(1) != all.films.getGenre(1));
            for (const auto* genres : {&translated.films, &translated.records})
                for (unsigned int i(0); i < genres->size(); ++i)
                    BOOST_TEST_CONTEXT("Genre " << i) {
                        const Glib::ustring genre(genres->getGenre(i));
                        BOOST_TEST(!genre.empty());
                        BOOST_TEST(genre.raw().front() != ' ');
                        BOOST_TEST(genre.raw().back() != ' ');
                    }
        }
    }
}
