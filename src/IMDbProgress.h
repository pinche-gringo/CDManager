#ifndef IMDBPROGRESS_H
#define IMDBPROGRESS_H

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
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <glibmm/ustring.h>

#include <gtkmm/progressbar.h>

namespace boost::json {
class value;
}

/**Class reading data from IMDb while showing its status in itself (a
 * progress bar)
 *
 * The data is queried (via HTTPS) from the GraphQL-API of IMDb.com, the
 * poster is loaded from the URL passed in the result.
 *
 * Available signals:
 *   - sigError: Emitted if an error occurs; the error-message is passed
 *   - sigAmbigous: Emitted if a search finds more than one film;
                    Passes a list of matching entries
 *   - sigSuccess: Emitted if a (single) film was found; Passes the
                   director, the film (with year in parenthesises), the genre,
                   the summary, the URL of the poster and the actors
 *   - sigIcon: Emitted after loading a poster; Passes the (JPEG) image
 * \remarks Don't destroy the object within the signal-callbacks
 */
class IMDbProgress : public Gtk::ProgressBar {
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

    IMDbProgress();
    IMDbProgress(const Glib::ustring& film);
    ~IMDbProgress() override;

    void start(const Glib::ustring& identifier, bool isImage = false);
    void stop();
    void disconnect();

    sigc::signal<void(const Glib::ustring&)> sigError;
    sigc::signal<void(const IMDbMatchData&)> sigAmbiguous;
    sigc::signal<void(const IMDbEntry&)> sigSuccess;
    sigc::signal<void(const std::string&)> sigIcon;

    IMDbProgress(const IMDbProgress& other) = delete;
    const IMDbProgress& operator=(const IMDbProgress& other) = delete;

    static std::string getIMDbID(const Glib::ustring& identifier);
    static std::string getSmallPoster(const std::string& url);

  protected:
    sigc::connection conProgress;

  private:
    struct ConnectInfo;

    void reStart(const std::string& idFilm);

    bool indicateWait();
    void error(const Glib::ustring& msg);

    void sendQuery(std::string_view query, std::string_view variable, const Glib::ustring& value);
    void sendRequest();
    void received(const std::string& response);
    void readSearch(const boost::json::value& response);
    [[nodiscard]] bool isExactMatch(const boost::json::value& film) const pre(data);
    void readFilm(const boost::json::value& response);

    std::unique_ptr<ConnectInfo> data;

    enum { NONE, SEARCH, TITLE, IMAGE } status {NONE};
};

#endif
