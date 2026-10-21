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

#include <string>
#include <string_view>
#include <vector>

#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>

#include "IMDbParser.h"

#define BOOST_TEST_MODULE IMDbParser
#include "UnitTest.h"

namespace {

/// Search result for "The Thing" (shortened, like returned by IMDb)
constexpr std::string_view SEARCH_THING(R"({"data": {"mainSearch": {"edges": [
  {"node": {"entity": {"id": "tt0084787", "titleText": {"text": "The Thing"}, "originalTitleText": {"text": "The Thing"},
                       "releaseYear": {"year": 1982}, "titleType": {"id": "movie", "text": "Movie"}}}},
  {"node": {"entity": {"id": "tt0905372", "titleText": {"text": "The Thing"}, "originalTitleText": {"text": "The Thing"},
                       "releaseYear": {"year": 2011}, "titleType": {"id": "movie", "text": "Movie"}}}},
  {"node": {"entity": {"id": "tt0044121", "titleText": {"text": "The Thing from Another World"},
                       "originalTitleText": {"text": "The Thing from Another World"},
                       "releaseYear": {"year": 1951}, "titleType": {"id": "movie", "text": "Movie"}}}},
  {"node": {"entity": {"id": "tt1234567", "titleText": {"text": "The Thing"}, "originalTitleText": {"text": "The Thing"},
                       "releaseYear": {"year": 2002}, "titleType": {"id": "videoGame", "text": "Video Game"}}}},
  {"node": {"entity": {"id": "tt7654321", "titleText": {"text": "The Thing: Making of"},
                       "originalTitleText": {"text": "The Thing: Making of"},
                       "releaseYear": null, "titleType": {"id": "video", "text": "Video"}}}}
]}}})");

/// Title-query for "Alien" (shortened, like returned by IMDb)
constexpr std::string_view FILM_ALIEN(R"({"data": {"title": {
  "titleText": {"text": "Alien"}, "releaseYear": {"year": 1979},
  "primaryImage": {"url": "https://m.media-amazon.com/images/M/MV5BMmQ2MmU3NzktZjAxOC00ZDZhLTk4YzEtMDMyMzcxY2IwMDAyXkEyXkFqcGc@._V1_.jpg"},
  "genres": {"genres": [{"text": "Horror"}, {"text": "Sci-Fi"}]},
  "plot": {"plotText": {"plainText": "The crew of a commercial spacecraft encounters a deadly lifeform."}},
  "summaries": {"edges": [{"node": {"plotText": {"plainText": "In deep space, the crew of the commercial starship Nostromo is awakened ..."}}}]},
  "directors": {"edges": [{"node": {"name": {"nameText": {"text": "Ridley Scott"}}}}]},
  "cast": {"edges": [{"node": {"name": {"nameText": {"text": "Sigourney Weaver"}}}},
                     {"node": {"name": {"nameText": {"text": "Tom Skerritt"}}}},
                     {"node": {"name": null}},
                     {"node": {"name": {"nameText": {"text": "John Hurt"}}}}]}
}}})");

boost::json::value parse(std::string_view json) { return boost::json::parse(json); }

/// Returns the IDs of the passed search entries
std::vector<std::string> ids(const IMDbParser::IMDbSearchEntries& entries) {
    std::vector<std::string> ids;
    for (const auto& entry : entries)
        ids.push_back(entry.url);
    return ids;
}

/// Returns the titles of the passed search entries
std::vector<std::string> titles(const IMDbParser::IMDbSearchEntries& entries) {
    std::vector<std::string> titles;
    for (const auto& entry : entries)
        titles.push_back(entry.title);
    return titles;
}

/// Returns a search result containing the passed entities
boost::json::value searchResult(std::initializer_list<std::string_view> entities) {
    std::string json(R"({"data": {"mainSearch": {"edges": [)");
    for (auto entity : entities) {
        if (json.back() == '}')
            json += ',';
        json += R"({"node": {"entity": )";
        json += entity;
        json += "}}";
    }
    return parse(json + "]}}}");
}

} // namespace

BOOST_AUTO_TEST_SUITE(IMDbID)

BOOST_AUTO_TEST_CASE(FromURL) {
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/title/tt0078748/") == "tt0078748");
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/title/tt0078748") == "tt0078748");
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/title/tt0078748/?ref_=fn_al_tt_1") == "tt0078748");
    BOOST_TEST(IMDbParser::getIMDbID("https://m.imdb.com/de/title/tt12345678/reviews") == "tt12345678");
    BOOST_TEST(IMDbParser::getIMDbID("http://imdb.com/title/tt0078748?x=1") == "tt0078748");
}

BOOST_AUTO_TEST_CASE(InvalidURL) {
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/title/tt/") == "");
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/title/ttabc/") == "");
    BOOST_TEST(IMDbParser::getIMDbID("https://www.imdb.com/name/nm0000631/") == "");
    BOOST_TEST(IMDbParser::getIMDbID("www.imdb.com/title/tt0078748") == "");
}

BOOST_AUTO_TEST_CASE(FromID) {
    BOOST_TEST(IMDbParser::getIMDbID("tt0078748") == "tt0078748");
    BOOST_TEST(IMDbParser::getIMDbID("tt12345678") == "tt12345678");
    BOOST_TEST(IMDbParser::getIMDbID("tt") == "");
    BOOST_TEST(IMDbParser::getIMDbID("tt12a") == "");
}

BOOST_AUTO_TEST_CASE(FromNumber) {
    BOOST_TEST(IMDbParser::getIMDbID("78748") == "tt0078748");
    BOOST_TEST(IMDbParser::getIMDbID("1234567") == "tt1234567");
    BOOST_TEST(IMDbParser::getIMDbID("12345678") == "tt12345678");
    BOOST_TEST(IMDbParser::getIMDbID("1984") == "tt0001984");
}

BOOST_AUTO_TEST_CASE(FromName) {
    BOOST_TEST(IMDbParser::getIMDbID("Alien") == "");
    BOOST_TEST(IMDbParser::getIMDbID("\"1984\"") == "");
    BOOST_TEST(IMDbParser::getIMDbID("2001: A Space Odyssey") == "");
    BOOST_TEST(IMDbParser::getIMDbID("") == "");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Poster)

BOOST_AUTO_TEST_CASE(SmallPoster) {
    BOOST_TEST(IMDbParser::getSmallPoster("https://m.media-amazon.com/images/M/MV5Babc@._V1_.jpg")
               == "https://m.media-amazon.com/images/M/MV5Babc@._V1_SY256_.jpg");
}

BOOST_AUTO_TEST_CASE(ReplacesModifiers) {
    BOOST_TEST(IMDbParser::getSmallPoster("https://m.media-amazon.com/images/M/MV5Babc@._V1_UX182_CR0,0,182,268_AL_.png")
               == "https://m.media-amazon.com/images/M/MV5Babc@._V1_SY256_.png");
}

BOOST_AUTO_TEST_CASE(OtherURLsAreKept) {
    BOOST_TEST(IMDbParser::getSmallPoster("https://example.com/poster.jpg") == "https://example.com/poster.jpg");
    BOOST_TEST(IMDbParser::getSmallPoster("https://example.com/poster._V1_") == "https://example.com/poster._V1_");
    BOOST_TEST(IMDbParser::getSmallPoster("") == "");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SearchTerm)

BOOST_AUTO_TEST_CASE(Plain) {
    const auto search(IMDbParser::getSearchTerm("The Thing"));
    BOOST_TEST(search.term == "The Thing");
    BOOST_TEST(search.title == "the thing");
    BOOST_TEST(search.year == 0);
}

BOOST_AUTO_TEST_CASE(WithYear) {
    const auto search(IMDbParser::getSearchTerm("The Thing (1982)"));
    BOOST_TEST(search.term == "The Thing");
    BOOST_TEST(search.title == "the thing");
    BOOST_TEST(search.year == 1982);
}

BOOST_AUTO_TEST_CASE(NoYear) {
    for (const char* term : {"The Thing(1982)", "The Thing (82)", "The Thing (abcd)", "The Thing (1982", "(1982)"})
        BOOST_TEST_CONTEXT(term) {
            const auto search(IMDbParser::getSearchTerm(term));
            BOOST_TEST(search.term == term);
            BOOST_TEST(search.year == 0);
        }
}

BOOST_AUTO_TEST_CASE(Quoted) {
    const auto search(IMDbParser::getSearchTerm("\"1984\""));
    BOOST_TEST(search.term == "1984");
    BOOST_TEST(search.title == "1984");
    BOOST_TEST(search.year == 0);

    BOOST_TEST(IMDbParser::getSearchTerm("\"Alien (1979)\"").term == "Alien (1979)");
    BOOST_TEST(IMDbParser::getSearchTerm("\"\"").term == "\"\"");
}

BOOST_AUTO_TEST_CASE(CaseFolded) {
    BOOST_TEST(IMDbParser::getSearchTerm("DER UNTERGANG").title == "der untergang");
    BOOST_TEST(IMDbParser::getSearchTerm("Amélie (2001)").title == "amélie");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Errors)

BOOST_AUTO_TEST_CASE(GetError) {
    BOOST_TEST(IMDbParser::getError(parse(R"({"errors": [{"message": "Bad query"}, {"message": "Other"}]})")) == "Bad query");
    BOOST_TEST(IMDbParser::getError(parse(R"({"errors": [{"message": "Bad"}], "data": null})")) == "Bad");
}

BOOST_AUTO_TEST_CASE(NoError) {
    BOOST_TEST(IMDbParser::getError(parse(R"({"errors": []})")) == "");
    BOOST_TEST(IMDbParser::getError(parse(R"({"errors": null})")) == "");
    BOOST_TEST(IMDbParser::getError(parse(SEARCH_THING)) == "");
    BOOST_TEST(IMDbParser::getError(parse("[]")) == "");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Search)

BOOST_AUTO_TEST_CASE(Entries) {
    const auto result(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("Thing")));
    BOOST_REQUIRE(result.films.contains(IMDbParser::POPULAR));
    const auto& films(result.films.at(IMDbParser::POPULAR));

    // Video games are skipped; other types than movies are marked
    BOOST_TEST(ids(films) == (std::vector<std::string> {"tt0084787", "tt0905372", "tt0044121", "tt7654321"}),
               boost::test_tools::per_element());
    BOOST_TEST(titles(films)
                   == (std::vector<std::string> {"The Thing (1982)", "The Thing (2011)", "The Thing from Another World (1951)",
                                                 "The Thing: Making of - Video"}),
               boost::test_tools::per_element());
    BOOST_TEST(result.id == "");
}

BOOST_AUTO_TEST_CASE(NothingFound) {
    for (const char* json : {R"({"data": {"mainSearch": {"edges": []}}})", R"({"data": {"mainSearch": null}})", "{}"})
        BOOST_TEST_CONTEXT(json) {
            const auto result(IMDbParser::readSearch(parse(json), IMDbParser::getSearchTerm("Thing")));
            BOOST_REQUIRE(result.films.contains(IMDbParser::POPULAR));
            BOOST_TEST(result.films.at(IMDbParser::POPULAR).empty());
            BOOST_TEST(result.id == "");
        }
}

BOOST_AUTO_TEST_CASE(InvalidEntriesAreSkipped) {
    const auto result(IMDbParser::readSearch(
        searchResult({R"(null)", R"({"titleText": {"text": "Without ID"}})", R"({"id": "", "titleText": {"text": "Empty"}})",
                      R"({"id": "tt0000001", "titleText": {"text": "Valid"}, "titleType": {"id": "tvEpisode"}})",
                      R"({"id": "tt0000002", "titleText": {"text": "Valid"}})"}),
        IMDbParser::getSearchTerm("Other")));
    BOOST_TEST(ids(result.films.at(IMDbParser::POPULAR)) == std::vector<std::string> {"tt0000002"},
               boost::test_tools::per_element());
}

// A single result is loaded directly
BOOST_AUTO_TEST_CASE(SingleResult) {
    const auto result(IMDbParser::readSearch(
        searchResult({R"({"id": "tt0078748", "titleText": {"text": "Alien"}, "titleType": {"id": "movie"}})"}),
        IMDbParser::getSearchTerm("Something else")));
    BOOST_TEST(result.id == "tt0078748");
}

// Regression (IMDbProgress: Fix importing by name): A single exact match is loaded directly
BOOST_AUTO_TEST_CASE(ExactMatch) {
    const auto result(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("The Thing from another world")));
    BOOST_TEST(result.id == "tt0044121");
    BOOST_TEST(result.films.at(IMDbParser::POPULAR).size() == 4U);
}

BOOST_AUTO_TEST_CASE(ExactMatchWithYear) {
    BOOST_TEST(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("The Thing (1982)")).id == "tt0084787");
    BOOST_TEST(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("the thing (2011)")).id == "tt0905372");
    BOOST_TEST(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("The Thing (1999)")).id == "");
}

// Several films with the same title (and no year): Let the user decide
BOOST_AUTO_TEST_CASE(SeveralExactMatches) {
    BOOST_TEST(IMDbParser::readSearch(parse(SEARCH_THING), IMDbParser::getSearchTerm("The Thing")).id == "");
}

// The localised title might differ from the original one; both are matched
BOOST_AUTO_TEST_CASE(OriginalTitle) {
    const auto json(searchResult(
        {R"({"id": "tt0363163", "titleText": {"text": "Downfall"}, "originalTitleText": {"text": "Der Untergang"},
             "releaseYear": {"year": 2004}, "titleType": {"id": "movie"}})",
         R"({"id": "tt0000001", "titleText": {"text": "Der Untergang der Titanic"}, "titleType": {"id": "movie"}})"}));
    BOOST_TEST(IMDbParser::readSearch(json, IMDbParser::getSearchTerm("Der Untergang")).id == "tt0363163");
    BOOST_TEST(IMDbParser::readSearch(json, IMDbParser::getSearchTerm("downfall")).id == "tt0363163");
}

// If several titles match exactly, but only one of them is a feature film, this is loaded
BOOST_AUTO_TEST_CASE(PreferMovies) {
    const auto json(searchResult(
        {R"({"id": "tt0000001", "titleText": {"text": "Alien"}, "titleType": {"id": "short", "text": "Short"}})",
         R"({"id": "tt0078748", "titleText": {"text": "Alien"}, "titleType": {"id": "movie", "text": "Movie"}})",
         R"({"id": "tt0000002", "titleText": {"text": "Alien"}, "titleType": {"id": "video", "text": "Video"}})",
         R"({"id": "tt0000003", "titleText": {"text": "Aliens"}, "titleType": {"id": "movie", "text": "Movie"}})"}));
    const auto result(IMDbParser::readSearch(json, IMDbParser::getSearchTerm("Alien")));
    BOOST_TEST(result.id == "tt0078748");
    BOOST_TEST(titles(result.films.at(IMDbParser::POPULAR))
                   == (std::vector<std::string> {"Alien - Short", "Alien", "Alien - Video", "Aliens"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Film)

BOOST_AUTO_TEST_CASE(ReadFilm) {
    const auto entry(IMDbParser::readFilm(parse(FILM_ALIEN)));
    BOOST_TEST(entry.title == "Alien (1979)");
    BOOST_TEST(entry.director == "Ridley Scott");
    BOOST_TEST(entry.genre == "Horror");
    BOOST_TEST(entry.summary == "In deep space, the crew of the commercial starship Nostromo is awakened ...");
    BOOST_TEST(entry.image
               == "https://m.media-amazon.com/images/M/MV5BMmQ2MmU3NzktZjAxOC00ZDZhLTk4YzEtMDMyMzcxY2IwMDAyXkEyXkFqcGc@._V1_SY256_.jpg");
    BOOST_TEST(std::vector<std::string>(entry.actors.begin(), entry.actors.end())
                   == (std::vector<std::string> {"Sigourney Weaver", "Tom Skerritt", "John Hurt"}),
               boost::test_tools::per_element());
}

// Without a summary the (short) plot is used
BOOST_AUTO_TEST_CASE(PlotAsSummary) {
    const auto entry(IMDbParser::readFilm(parse(R"({"data": {"title": {"titleText": {"text": "Alien"},
        "plot": {"plotText": {"plainText": "Short plot"}}, "summaries": {"edges": []}}}})")));
    BOOST_TEST(entry.summary == "Short plot");
}

// Missing information is left empty
BOOST_AUTO_TEST_CASE(MinimalFilm) {
    const auto entry(IMDbParser::readFilm(parse(R"({"data": {"title": {"titleText": {"text": "Alien"}, "releaseYear": null,
        "primaryImage": null, "genres": {"genres": []}, "plot": null, "directors": {"edges": []}, "cast": null}}})")));
    BOOST_TEST(entry.title == "Alien");
    BOOST_TEST(entry.director == "");
    BOOST_TEST(entry.genre == "");
    BOOST_TEST(entry.summary == "");
    BOOST_TEST(entry.image == "");
    BOOST_TEST(entry.actors.empty());
}

// An unknown ID returns no title
BOOST_AUTO_TEST_CASE(UnknownFilm) {
    for (const char* json : {R"({"data": {"title": null}})", R"({"data": null})", "{}"})
        BOOST_TEST_CONTEXT(json) {
            BOOST_TEST(IMDbParser::readFilm(parse(json)).title == "");
        }
}

BOOST_AUTO_TEST_SUITE_END()
