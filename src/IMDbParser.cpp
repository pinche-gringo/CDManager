// PROJECT     : CDManager
// SUBSYSTEM   : Films
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 21.10.2026
// COPYRIGHT   : Copyright (C) 2010 - 2013, 2026

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
#include <array>
#include <string_view>

#include <boost/json/value.hpp>

#include <YGP/Trace.h>

#include "IMDbParser.h"

namespace {

/// Image modifier making IMDb deliver posters with a height of 256 pixels
constexpr std::string_view POSTER_SIZE("SY256_");
constexpr std::string_view POSTER_MODIFIERS("._V1_");

/// Types of titles, which are no films (on a disk) and therefore skipped in search results
constexpr std::array SKIPPED_TYPES {"podcastEpisode", "podcastSeries", "tvEpisode", "videoGame", "musicVideo"};

//-----------------------------------------------------------------------------
/// Returns the value at the passed JSON pointer (RFC 6901)
/// \param root JSON value to search in
/// \param pointer Path to the value (like "/data/title")
/// \returns const boost::json::value* Found value or nullptr, if not found or null
//-----------------------------------------------------------------------------
const boost::json::value* find(const boost::json::value& root, std::string_view pointer) {
    boost::system::error_code err;
    const boost::json::value* value(root.find_pointer(pointer, err));
    return (value && !value->is_null()) ? value : nullptr;
}

//-----------------------------------------------------------------------------
/// Returns the text at the passed JSON pointer
/// \param root JSON value to search in
/// \param pointer Path to the text
/// \returns Glib::ustring Found text or an empty string
//-----------------------------------------------------------------------------
Glib::ustring getText(const boost::json::value& root, std::string_view pointer) {
    const boost::json::value* value(find(root, pointer));
    return (value && value->is_string()) ? Glib::ustring(std::string(value->get_string())) : Glib::ustring();
}

//-----------------------------------------------------------------------------
/// Returns the array at the passed JSON pointer
/// \param root JSON value to search in
/// \param pointer Path to the array
/// \returns const boost::json::array* Found array or nullptr
//-----------------------------------------------------------------------------
const boost::json::array* getArray(const boost::json::value& root, std::string_view pointer) {
    const boost::json::value* value(find(root, pointer));
    return (value && value->is_array()) ? &value->get_array() : nullptr;
}

//-----------------------------------------------------------------------------
/// Returns the title of a film with its year in parenthesis appended (if known)
/// \param film JSON object describing the film
/// \returns Glib::ustring Title (with year) or an empty string
//-----------------------------------------------------------------------------
Glib::ustring getTitle(const boost::json::value& film) {
    Glib::ustring title(getText(film, "/titleText/text"));
    const boost::json::value* year(find(film, "/releaseYear/year"));
    if (!title.empty() && year && year->is_int64())
        title += " (" + std::to_string(year->get_int64()) + ')';
    return title;
}

//-----------------------------------------------------------------------------
/// Checks if the passed character is a digit
/// \param ch Character to check
/// \returns bool True, if ch is a digit
//-----------------------------------------------------------------------------
bool isDigit(char ch) { return (ch >= '0') && (ch <= '9'); }

} // namespace

//-----------------------------------------------------------------------------
/// Extracts the IMDb ID out of the passed identification of a film:
///   - If it is an URL containing /title/tt<number>, the tt<number> part
///   - If it is "tt" followed by digits consider it an IMDb-ID
///   - If it is a number consider it an IMDb-ID (without the leading tt)
/// \param identifier Identification of a film
/// \returns std::string IMDb ID (tt followed by at least 7 digits) or an empty string, if identifier
///          is not an ID (but e.g. a name of a film)
//-----------------------------------------------------------------------------
std::string IMDbParser::getIMDbID(const Glib::ustring& identifier) {
    const std::string& id(identifier.raw());

    if (const auto pos(id.find("/title/tt")); (pos != std::string::npos) && (id.find("://") != std::string::npos)) {
        const auto start(pos + 7);
        const auto end(std::find_if_not(id.begin() + start + 2, id.end(), isDigit) - id.begin());
        return (end > static_cast<long>(start + 2)) ? id.substr(start, end - start) : std::string();
    }
    if ((id.size() > 2) && id.starts_with("tt") && std::ranges::all_of(id.substr(2), isDigit))
        return id;
    if (!id.empty() && std::ranges::all_of(id, isDigit))
        return "tt" + std::string((id.size() < 7) ? (7 - id.size()) : 0, '0') + id;
    return {};
}

//-----------------------------------------------------------------------------
/// Converts the URL of a poster into the URL of a small version of it.
///
/// Poster-URLs look like https://m.media-amazon.com/images/M/<id>._V1_.jpg; the part
/// between ._V1_ and the extension specifies modifications (like scaling) of the image.
/// \param url URL of the poster
/// \returns std::string URL of the poster with a height of 256 pixels
//-----------------------------------------------------------------------------
std::string IMDbParser::getSmallPoster(const std::string& url) {
    const auto start(url.rfind(POSTER_MODIFIERS));
    const auto ext(url.rfind('.'));
    if ((start == std::string::npos) || (ext < start + POSTER_MODIFIERS.size()))
        return url;
    return url.substr(0, start + POSTER_MODIFIERS.size()) + std::string(POSTER_SIZE) + url.substr(ext);
}

//-----------------------------------------------------------------------------
/// Converts the name of a film into the term to search for:
///   - A name within quotes is searched as is (without the quotes)
///   - A trailing year in parenthesis (like "(2026)") is removed from the term;
///     it is used to detect an exact match
/// \param identifier Name of the film
/// \returns SearchTerm Term to search for
//-----------------------------------------------------------------------------
IMDbParser::SearchTerm IMDbParser::getSearchTerm(const Glib::ustring& identifier) {
    SearchTerm search {identifier};
    if ((search.term.size() > 2) && search.term.raw().starts_with('"') && search.term.raw().ends_with('"'))
        search.term = search.term.substr(1, search.term.size() - 2);
    else if (const std::string& raw(identifier.raw()); (raw.size() > 7) && raw.ends_with(')') && raw.substr(raw.size() - 7, 2) == " ("
             && std::ranges::all_of(raw.substr(raw.size() - 5, 4), isDigit)) {
        search.year = std::stol(raw.substr(raw.size() - 5, 4));
        search.term = raw.substr(0, raw.size() - 7);
    }
    search.title = search.term.casefold();
    return search;
}

//-----------------------------------------------------------------------------
/// Returns the (first) error reported in a response
/// \param response Response of IMDb.com
/// \returns Glib::ustring Error message or an empty string, if the response contains no error
//-----------------------------------------------------------------------------
Glib::ustring IMDbParser::getError(const boost::json::value& response) {
    const boost::json::array* errors(getArray(response, "/errors"));
    return (errors && !errors->empty()) ? getText(errors->front(), "/message") : Glib::ustring();
}

//-----------------------------------------------------------------------------
/// Checks if the passed film matches the searched title (and year) exactly
/// \param film JSON object describing the film
/// \param search Searched film
/// \returns bool True, if the (localised or original) title matches (ignoring the case)
//-----------------------------------------------------------------------------
bool IMDbParser::isExactMatch(const boost::json::value& film, const SearchTerm& search) {
    if (search.year) {
        const boost::json::value* year(find(film, "/releaseYear/year"));
        if (!year || !year->is_int64() || (year->get_int64() != search.year))
            return false;
    }
    return (getText(film, "/titleText/text").casefold() == search.title)
           || (getText(film, "/originalTitleText/text").casefold() == search.title);
}

//-----------------------------------------------------------------------------
/// Extracts the results of a search. If it contains only one film or only one
/// film matches exactly, its ID is returned too
/// \param response Response of IMDb.com
/// \param search Searched film
/// \returns SearchResult Found films
//-----------------------------------------------------------------------------
IMDbParser::SearchResult IMDbParser::readSearch(const boost::json::value& response, const SearchTerm& search) {
    SearchResult result;
    IMDbSearchEntries& entries(result.films[POPULAR]);
    std::vector<Glib::ustring> exactMatches;
    std::vector<Glib::ustring> exactMovies; ///< Exact matches, which are feature films (no shorts, videos, ...)

    if (const boost::json::array* edges = getArray(response, "/data/mainSearch/edges"))
        for (const auto& edge : *edges)
            if (const boost::json::value* film = find(edge, "/node/entity")) {
                const Glib::ustring id(getText(*film, "/id"));
                if (id.empty())
                    continue;

                const Glib::ustring type(getText(*film, "/titleType/id"));
                if (std::ranges::contains(SKIPPED_TYPES, type.raw()))
                    continue;

                Glib::ustring name(getTitle(*film));
                if (!type.empty() && (type != "movie"))
                    name += " - " + getText(*film, "/titleType/text");
                TRACE8("IMDbParser::readSearch (const boost::json::value&, const SearchTerm&) - " << id << ": " << name);
                entries.emplace_back(id, name);
                if (isExactMatch(*film, search)) {
                    exactMatches.push_back(id);
                    if (type == "movie")
                        exactMovies.push_back(id);
                }
            }
    TRACE5("IMDbParser::readSearch (const boost::json::value&, const SearchTerm&) - Films: " << entries.size() << "; exact: "
           << exactMatches.size() << '/' << exactMovies.size());

    // Load the film directly, if it is the only one found or the only one matching exactly
    // (with preference to feature films, as there are often shorts or videos with the same title)
    if (entries.size() == 1)
        result.id = entries.front().url;
    else if (exactMatches.size() == 1)
        result.id = exactMatches.front();
    else if (exactMovies.size() == 1)
        result.id = exactMovies.front();
    return result;
}

//-----------------------------------------------------------------------------
/// Extracts information about a film from the response
/// \param response Response of IMDb.com
/// \returns IMDbEntry Read data; its title is empty, if the response contains no film
//-----------------------------------------------------------------------------
IMDbParser::IMDbEntry IMDbParser::readFilm(const boost::json::value& response) {
    IMDbEntry entry;
    if (const boost::json::value* film = find(response, "/data/title")) {
        entry.title = getTitle(*film);
        entry.director = getText(*film, "/directors/edges/0/node/name/nameText/text");
        entry.genre = getText(*film, "/genres/genres/0/text");
        entry.summary = getText(*film, "/summaries/edges/0/node/plotText/plainText");
        if (entry.summary.empty())
            entry.summary = getText(*film, "/plot/plotText/plainText");
        if (const Glib::ustring poster(getText(*film, "/primaryImage/url")); !poster.empty())
            entry.image = getSmallPoster(poster);

        if (const boost::json::array* cast = getArray(*film, "/cast/edges"))
            for (const auto& actor : *cast)
                if (Glib::ustring name(getText(actor, "/node/name/nameText/text")); !name.empty())
                    entry.actors.push_back(std::move(name));
    }

    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Director: " << entry.director);
    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Name: " << entry.title);
    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Genre: " << entry.genre);
    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Summary: " << entry.summary);
    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Icon: " << entry.image);
    TRACE1("IMDbParser::readFilm (const boost::json::value&) - Actors: " << entry.actors.size());
    return entry;
}
