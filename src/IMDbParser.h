#ifndef IMDBPARSER_H
#define IMDBPARSER_H

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

#include <list>
#include <map>
#include <string>
#include <vector>

#include <glibmm/ustring.h>

namespace boost::json {
class value;
}

/**Class interpreting the identification of a film and the (JSON) responses
 * of the GraphQL-API of IMDb.com.
 *
 * It holds no state and doesn't communicate itself (see IMDbProgress for that).
 */
class IMDbParser {
  public:
    enum match { POPULAR, EXACT, PARTIAL };
    struct IMDbSearchEntry {
        Glib::ustring url; ///< IMDb ID of the film (tt...)
        Glib::ustring title;

        IMDbSearchEntry(const Glib::ustring& url, const Glib::ustring& title) : url(url), title(title) {}
    };
    using IMDbSearchEntries = std::list<IMDbSearchEntry>;
    using IMDbMatchData = std::map<match, IMDbSearchEntries>;

    struct IMDbEntry {
        Glib::ustring director;
        Glib::ustring title;
        Glib::ustring genre;
        Glib::ustring summary;
        std::string image;
        std::vector<Glib::ustring> actors;
    };

    /// Searched film: The term to send to IMDb and what an exact match must look like
    struct SearchTerm {
        Glib::ustring term;  ///< Term to search for
        Glib::ustring title; ///< Searched title (case-folded), to detect exact matches
        long year{0};        ///< Searched year (or 0, if not specified)
    };

    /// Result of a search
    struct SearchResult {
        IMDbMatchData films; ///< Found films
        std::string id;      ///< ID of the film to load directly (if unique); else empty
    };

    [[nodiscard]] static std::string getIMDbID(const Glib::ustring& identifier);
    [[nodiscard]] static std::string getSmallPoster(const std::string& url);
    [[nodiscard]] static SearchTerm getSearchTerm(const Glib::ustring& identifier);

    [[nodiscard]] static Glib::ustring getError(const boost::json::value& response);
    [[nodiscard]] static SearchResult readSearch(const boost::json::value& response, const SearchTerm& search);
    [[nodiscard]] static IMDbEntry readFilm(const boost::json::value& response);

  private:
    IMDbParser() = delete("IMDbParser has only static members");

    [[nodiscard]] static bool isExactMatch(const boost::json::value& film, const SearchTerm& search);
};

#endif
