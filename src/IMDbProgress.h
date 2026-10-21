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

#include <memory>
#include <string>
#include <string_view>

#include <glibmm/ustring.h>

#include <gtkmm/progressbar.h>

#include "IMDbParser.h"

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
    using match = IMDbParser::match;
    using IMDbSearchEntry = IMDbParser::IMDbSearchEntry;
    using IMDbSearchEntries = IMDbParser::IMDbSearchEntries;
    using IMDbMatchData = IMDbParser::IMDbMatchData;
    using IMDbEntry = IMDbParser::IMDbEntry;

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
    void readFilm(const boost::json::value& response);

    std::unique_ptr<ConnectInfo> data;

    enum { NONE, SEARCH, TITLE, IMAGE } status {NONE};
};

#endif
