// PROJECT     : CDManager
// SUBSYSTEM   : Films
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 04.04.2010
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

#include <cctype>
#include <cstring>

#include <algorithm>
#include <array>
#include <istream>
#include <ostream>
#include <ranges>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>

#include <glibmm/main.h>

#include <YGP/ANumeric.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "IMDbProgress.h"

namespace {

constexpr std::string_view HOST("www.imdb.com");
constexpr std::string_view PORT("http");
constexpr std::string_view NOPOSTER("title_addposter.jpg");

constexpr std::string_view HTTP("http://");
constexpr std::string_view LINK("a href=\"/title/tt");
constexpr std::string_view LINE("<tr class=\"findResult");
constexpr std::string_view NAME("<td class=\"result_text\">");

//-----------------------------------------------------------------------------
/// Appends all data still available in the passed stream to the passed string
/// \param input Stream to read from
/// \param target String to append the data to
//-----------------------------------------------------------------------------
void appendRemaining(std::istream& input, std::string& target) {
    std::array<char, 256> buffer;
    do {
        input.read(buffer.data(), buffer.size());
        target.append(buffer.data(), input.gcount());
    }
    while (input);
}

} // namespace

struct IMDbProgress::ConnectInfo {
    boost::asio::io_context svcIO;
    boost::asio::ip::tcp::resolver resolver {svcIO};
    boost::asio::ip::tcp::socket sockIO {svcIO};
    boost::asio::streambuf buffer;

    std::string host {HOST};
    std::string path;
    std::string response;

    /// Constructor
    ConnectInfo(const Glib::ustring& url);
    ~ConnectInfo() {
        sockIO.close();
        svcIO.stop();
    }

  private:
    static bool isNumber(const Glib::ustring& nr);
    static std::string percentEncode(const Glib::ustring& data);
};

//-----------------------------------------------------------------------------
/// Constructor
/// \param id String identifying the film to load; can be an URL, an IMDb ID or a film name
//-----------------------------------------------------------------------------
IMDbProgress::ConnectInfo::ConnectInfo(const Glib::ustring& id) {
    Glib::ustring::size_type pos(Glib::ustring::npos);
    if (id.raw().starts_with(HTTP) && ((pos = id.find('/', HTTP.size() + 1)) != Glib::ustring::npos)) {
        // Starts with http:// and has a slash (/) separating host and path
        host = id.substr(HTTP.size(), pos - HTTP.size());
        path = id.substr(pos + 1);
    }
    else {
        // Doesn't seem to be an URL; so check for IMDb identifier or name
        if ((id.length() == 9) && !id.compare(0, 2, "tt") && isNumber(id.substr(2)))
            path = "title/" + id + '/';
        else if (isNumber(id) > 0)
            path = "title/tt" + std::string((id.length() < 7) ? (7 - id.length()) : 0, '0') + id + '/';
        else {
            path = percentEncode(id);
            std::ranges::replace(path, ' ', '+');

            path = "find?s=tt&q=" + path;
        }
    }
    TRACE1("ConnectInfo::ConnectInfo (const Glib::ustring&) - " << host << " - " << path);
}

//-----------------------------------------------------------------------------
/// Checks if the passed text is a number
/// \param nr Number to inspect
/// \returns bool True, if the passed text is a number
//-----------------------------------------------------------------------------
bool IMDbProgress::ConnectInfo::isNumber(const Glib::ustring& nr) {
    return std::ranges::all_of(nr, [](gunichar ch) { return (ch >= '0') && (ch <= '9'); });
}

//-----------------------------------------------------------------------------
/// Converts invalid characters for an URI to its percent-encoded counterparts
/// according to RFC 3986 (http://tools.ietf.org/html/rfc3986)
/// \param data Text to convert
/// \returns std::string Percent-encoded text
//-----------------------------------------------------------------------------
std::string IMDbProgress::ConnectInfo::percentEncode(const Glib::ustring& data) {
    constexpr std::string_view convTable("0123456789ABCDEF");

    // Work on the UTF-8 bytes: in UTF-8 every byte of a non-ASCII character is >= 0x80
    std::string result;
    for (const unsigned char ch : data.raw())
        if ((ch < 0x80) && (g_ascii_isalnum(ch) || std::string_view(".-_~").contains(static_cast<char>(ch))))
            result += static_cast<char>(ch);
        else {
            result += '%';
            result += convTable[(ch & 0xF0) >> 4];
            result += convTable[ch & 0x0F];
        }
    return result;
}

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress() : Gtk::ProgressBar() {
    TRACE5("IMDbProgress::IMDbProgress ()");
    set_show_text(); // GTK4 shows the text only on request
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param film ID of film to import
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress(const Glib::ustring& film) : Gtk::ProgressBar() {
    TRACE5("IMDbProgress::IMDbProgress (const Glib::ustring&) - " << film);
    set_show_text(); // GTK4 shows the text only on request
    start(film);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
IMDbProgress::~IMDbProgress() {
    TRACE6("IMDbProgress::~IMDbProgress ()");
    stop();
}

//-----------------------------------------------------------------------------
/// Starts the communication
/// \param identifier Film to load; this can be either its name, its
///                   number on IMDb.com or its whole URL
/// \param isImage Flag if identifier specifies an image
//-----------------------------------------------------------------------------
void IMDbProgress::start(const Glib::ustring& identifier, bool isImage) {
    TRACE1("IMDbProgress::start (const Glib::ustring&) - " << identifier);

    contract_assert(!data);
    contract_assert(status == NONE);
    if (isImage) {
        status = IMAGE;
        set_text(_("Loading icon ..."));
    }
    else {
        status = TITLE;
        set_text(_("Connecting to IMDb.com ..."));
    }
    data = std::make_unique<ConnectInfo>(identifier);
    pulse();

    connect();
    conPoll = Glib::signal_timeout().connect(sigc::mem_fun(*this, &IMDbProgress::poll), 50);

    conProgress = Glib::signal_timeout().connect(sigc::mem_fun(*this, &IMDbProgress::indicateWait), 150);
}

//-----------------------------------------------------------------------------
/// Stops the communication
/// \note It is not save to call this method while handling the callback of a signal
//-----------------------------------------------------------------------------
void IMDbProgress::stop() {
    TRACE3("IMDbProgress::stop ()");
    status = NONE;
    if (data) {
        disconnect();
        data.reset();
    }
}

//-----------------------------------------------------------------------------
/// Stops polling for events
//-----------------------------------------------------------------------------
void IMDbProgress::disconnect() {
    if (conPoll.connected())
        conPoll.disconnect();
    if (conProgress.connected())
        conProgress.disconnect();
    contract_assert(!conPoll.connected());
    contract_assert(!conProgress.connected());
}

//-----------------------------------------------------------------------------
/// Polls for available boost:asio-events
/// \returns bool Always true, indicating to continue with polling
//-----------------------------------------------------------------------------
bool IMDbProgress::poll() {
    TRACE9("IMDbProgress::poll ()");
    contract_assert(data);

    data->svcIO.poll();
    return true;
}

//-----------------------------------------------------------------------------
/// Updates the progress-bar while information is still loaded.
//-----------------------------------------------------------------------------
bool IMDbProgress::indicateWait() {
    contract_assert(data);
    if (data->response.size())
        set_text(Glib::ustring::compose(_("Receiving from IMDb.com: %1 KB"),
                                        Glib::ustring(YGP::ANumeric(data->response.size() >> 10).toString())));
    pulse();
    return true;
}

//-----------------------------------------------------------------------------
/// Opens a connection to IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::connect() {
    TRACE5("IMDbProgress::connect ()");
    contract_assert(data);

    data->resolver.async_resolve(
        data->host, PORT,
        [this](const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::results_type& results) {
            resolved(err, results.empty() ? boost::asio::ip::tcp::resolver::results_type::iterator() : results.begin());
        });
    data->svcIO.poll();
}

//-----------------------------------------------------------------------------
/// Callback after resolving the name of IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to available endpoints (in case of success)
//-----------------------------------------------------------------------------
void IMDbProgress::resolved(const boost::system::error_code& err,
                            boost::asio::ip::tcp::resolver::results_type::iterator iEndpoints) {
    TRACE7("IMDbProgress::resolved (boost::system::error_code&, iterator)");
    contract_assert(data);

    if (!err && (iEndpoints == boost::asio::ip::tcp::resolver::results_type::iterator()))
        error(boost::system::error_code(boost::asio::error::host_not_found).message());
    else if (!err) {
        // Attempt a connection to the first endpoint in the list. Each endpoint
        // will be tried until we successfully establish a connection.
        data->sockIO.async_connect(*iEndpoints, [this, iEndpoints](const boost::system::error_code& errConnect) {
            connected(errConnect, iEndpoints);
        });
    }
    else
        error(err.message());
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
/// \param msg Message to display
//-----------------------------------------------------------------------------
void IMDbProgress::error(const Glib::ustring& msg) {
    disconnect();
    sigError.emit(msg);
}

//-----------------------------------------------------------------------------
/// Callback after connecting to IMDb.com. Tries to load the film
/// specified at construction-time/by the last start()-call according
/// to the following algorithm:
///   - If it starts with http://www.imdb.com/ use the string as is
///   - If it is "tt" followed by (exactly) 7 digits consider it an IMDb-ID
///   - If it is a number consider it an IMDb-ID
///   - Else perform a search within all titles
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to remaining endpoints
/// \note To search for a film having a number as title (e.g. 1984) put it within quotes
//-----------------------------------------------------------------------------
void IMDbProgress::connected(const boost::system::error_code& err,
                             boost::asio::ip::tcp::resolver::results_type::iterator iEndpoints) {
    TRACE2("IMDbProgress::connected (boost::asio::streambuf*, boost::system::error_code&, iterator)");
    contract_assert(data);

    // The connection was successful. Send the request.
    if (!err)
        sendRequest();
    else {
        // The connection failed. Try the next endpoint in the list.
        data->sockIO.close();
        if (++iEndpoints != boost::asio::ip::tcp::resolver::results_type::iterator())
            resolved(boost::system::error_code(), iEndpoints);
        else
            resolved(err, iEndpoints); // No endpoints left: report the last error
    }
}

//-----------------------------------------------------------------------------
/// Sending the request about a title to IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::sendRequest() {
    contract_assert(data);
    std::ostream request(&data->buffer);

    TRACE7("IMDbProgress::sendRequest () - " << data->path);
    request << "GET /" << data->path << " HTTP/1.0\r\nHost: " << data->host << "\r\nAccept: */*\r\nConnection: close\r\n\r\n";

    boost::asio::async_write(data->sockIO, data->buffer,
                             [this](const boost::system::error_code& err, std::size_t) { requestWritten(err); });
}

//-----------------------------------------------------------------------------
/// Callback after having sent the HTTP-request
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::requestWritten(const boost::system::error_code& err) {
    TRACE1("IMDbProgress::requestWritten (const boost::system::error_code&)");
    contract_assert(data);

    if (!err) {
        // Read the response status line.
        boost::asio::async_read_until(data->sockIO, data->buffer, "\r\n",
                                      [this](const boost::system::error_code& errRead, std::size_t) { readStatus(errRead); });
    }
    else
        error(err.message());
}

//-----------------------------------------------------------------------------
/// Callback after reading the HTTP-status
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readStatus(const boost::system::error_code& err) {
    TRACE1("IMDbProgress::readStatus (boost::system::error_code&)");
    contract_assert(data);

    if (!err) {
        // Check that response is OK.
        std::istream response(&data->buffer);
        std::string idHTTP;
        unsigned int nrStatus;
        std::string msgStatus;

        response >> idHTTP >> nrStatus;
        std::getline(response, msgStatus);

        if (!response || !idHTTP.starts_with("HTTP/")) {
            error(Glib::ustring::compose(_("Invalid response: `%1'"), Glib::ustring(idHTTP)));
            return;
        }

        TRACE1("Status " << nrStatus);
        switch (nrStatus) {
        case 200: // HTTP OK
            break;

        case 302: { // HTTP Moved temporarily
            std::string url, loc;
            char ch;

            do {
                response >> loc >> ch;
                response.putback(ch);
                std::getline(response, url);
                TRACE8("IMDbProgress::readStatus (boost::system::error_code&) - Location: " << url << '/' << url.length());
            }
            while (response && !loc.starts_with("Location:"));
            // Strip trailing whitespaces
            while (url.length() && isspace(static_cast<unsigned char>(url.back()))) {
                TRACE1("Removing " << url.back());
                url.pop_back();
            }
            if (response && url.length()) {
                contract_assert(url[url.length() - 1]);
                TRACE1("Size " << url.length());
                Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*this, &IMDbProgress::reStart), url));
            }
            else
                error(_("HTTP status code 302 does not contain a location"));
            return;
        }

        default: // Other error
            error(Glib::ustring::compose(_("IMDb.com returned status code %1 %2"),
                                         Glib::ustring(YGP::ANumeric(nrStatus).toString()), Glib::ustring(msgStatus)));
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::async_read_until(data->sockIO, data->buffer, "\r\n\r\n",
                                      [this](const boost::system::error_code& errRead, std::size_t) { readHeaders(errRead); });
    }
    else
        error(err.message());
}

//-----------------------------------------------------------------------------
/// Callback after reading the headers
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readHeaders(const boost::system::error_code& err) {
    TRACE4("IMDbProgress::readHeaders (boost::system::error_code&)");
    contract_assert(data);

    if (!err) {
        // Skip the response headers.
        std::istream response(&data->buffer);
        std::string line;
        while (std::getline(response, line) && (line != "\r"))
            ;

        // Read the remaining content
        appendRemaining(response, data->response);

        boost::asio::async_read(data->sockIO, data->buffer, boost::asio::transfer_at_least(1),
                                [this](const boost::system::error_code& errRead, std::size_t) { readContent(errRead); });
    }
    else
        error(err.message());
}

//-----------------------------------------------------------------------------
/// Callback after reading (a part of) the content. The parsed content
/// is analysed if it contains a matching film or IMDb's search page.
///
/// Depending on the content either the film-information is extracted
/// and listeners are informed about the extracted data or the most
/// likely results of the search are extracted and the listeners are
/// informed about them.
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readContent(const boost::system::error_code& err) {
    TRACE9("IMDbProgress::readContent (boost::system::error_code&)");
    contract_assert(data);

    Glib::ustring msg;
    if (!err) {
        std::istream response(&data->buffer);
        appendRemaining(response, data->response);

        // Continue reading remaining data until EOF.
        boost::asio::async_read(data->sockIO, data->buffer, boost::asio::transfer_at_least(1),
                                [this](const boost::system::error_code& errRead, std::size_t) { readContent(errRead); });
        return;
    }
    else if (err == boost::asio::error::eof) {
        if (status == TITLE) {
            readFilm(msg);
            if (msg.empty())
                return;
        }
        else {
            readImage();
            return;
        }
    }
    else
        msg = err.message();

    error(msg);
}

//-----------------------------------------------------------------------------
/// Reads the icon from the connection and emits a signal
//-----------------------------------------------------------------------------
void IMDbProgress::readImage() {
    disconnect();
    sigIcon.emit(data->response);
}

//-----------------------------------------------------------------------------
/// Extracts information about a film from the read input and emits a signal
/// informing about the read data
/// \param msg String to write an error message into, if any
//-----------------------------------------------------------------------------
void IMDbProgress::readFilm(Glib::ustring& msg) {
    msg.clear();
    std::string name(extract("<head>", nullptr, "<title>", "</title>"));
    TRACE4("IMDbProgress::readFilm (boost::system::error_code&) - Final: " << name << ": " << data->response.size());

    if (name == "Find - IMDb") { // IMDb's search page found
        IMDbMatchData films;
        unsigned int cFilms(0);

        static constexpr std::array<std::string_view, 1> sections {"<a name=\"tt\"></a>Titles<"};
        for (const auto [i, section] : std::views::enumerate(sections)) {
            IMDbSearchEntries& entries(films[static_cast<match>(i)]);
            extractSearch(entries, data->response, section);
            cFilms += entries.size();
        }
        TRACE5("Films: " << cFilms);

        if (!cFilms)
            msg = _("IMDb didn't find any matching films!");
        else if (cFilms == 1) {
            for (const auto& [_, entries] : films)
                if (entries.size())
                    Glib::signal_idle().connect_once(
                        sigc::bind(sigc::mem_fun(*this, &IMDbProgress::reStart), entries.begin()->url));
        }
        else {
            disconnect();
            sigAmbiguous.emit(films);
        }
    }
    else {
        if (name.length() >= 7) // Strip " - IMDb"
            name.erase(name.length() - 7);
        std::string director(extract("Director:", " href=\"/name/nm", "name\">", "</span>"));
        std::string genre(extract("<a href=\"/genre/", nullptr, "<span class=\"itemprop\" itemprop=\"genre\">", "</span>"));
        std::string summary(extract("<h2>Storyline</h2>", nullptr, "<p>", "<em class="));
        std::string image(extract("img_primary", "<img", "src=\"", "\""));
        YGP::convertHTML2UTF8(director);
        YGP::convertHTML2UTF8(genre);
        YGP::convertHTML2UTF8(name);
        YGP::convertHTML2UTF8(summary);

        TRACE1("IMDbProgress::readFilm (boost::system::error_code&) - Director: " << director);
        TRACE1("IMDbProgress::readFilm (boost::system::error_code&) - Name: " << name);
        TRACE1("IMDbProgress::readFilm (boost::system::error_code&) - Genre: " << genre);
        TRACE1("IMDbProgress::readFilm (boost::system::error_code&) - Summary: " << summary);
        TRACE1("IMDbProgress::readFilm (boost::system::error_code&) - Icon: " << image);

        if (director.size() || name.size()) {
            if (image.ends_with(NOPOSTER))
                image.clear();
            IMDbEntry entry(director, name, genre, summary, image);
            disconnect();
            sigSuccess(entry);
        }
        else
            msg = _("Couldn't extract film-information from IMDb! Maybe the site was redesigned ...");
    }
}

//-----------------------------------------------------------------------------
/// Extracts a substring out of (a previously parsed) response
/// \param section Section in response which is followed by the searched text
/// \param subpart Text leading to the searched text
/// \param before Text immediately before the searched text
/// \param after Text immediately after the searched text
//-----------------------------------------------------------------------------
Glib::ustring IMDbProgress::extract(const char* section, const char* subpart, const char* before, const char* after) const {
    contract_assert(data);

    std::string::size_type i(data->response.find(section));
    if (i != std::string::npos)
        if (!subpart || ((i = data->response.find(subpart, i)) != std::string::npos))
            if ((i = data->response.find(before, i)) != std::string::npos) {
                i += strlen(before);

                // Skip white-space at beginning
                i = data->response.find_first_not_of(" \t\n\r", i);
                std::string::size_type end(data->response.find(after, i));
                if (end != std::string::npos) {
                    end = data->response.find_last_not_of(" \t\n\r", end - 1);
                    return std::string(data->response.substr(i, end - i + 1));
                }
            }
    return std::string();
}

//-----------------------------------------------------------------------------
/// Tries to extract IMDb-search results from the passed text. Found entries are
/// added to the passed list (with the ID as url and the name as title).
/// \param target List where the found entries are written to
/// \param src String to revise (HTML page read from IMDb)
/// \param section Text for section heading the entries
//-----------------------------------------------------------------------------
void IMDbProgress::extractSearch(IMDbSearchEntries& target, const std::string& src, std::string_view section) {
    std::string::size_type start(src.find(section));
    std::string::size_type end(src.find("</table>", start + 10));
    TRACE1("IMDbProgress::extractSearch (std::map&, const std::string&) - Search from " << start << '-' << end);

    while (start < end) {
        // Align with lines of result
        start = src.find(LINE, start);
        if ((start != std::string::npos) && (start < end)) {
            // Skip to the name column
            start = src.find(NAME, start + LINE.size());
            if (start == std::string::npos)
                return;

            // Extract the 7 digit long ID from the contained link
            start = src.find(LINK, start + NAME.size() + 1);
            if (start != std::string::npos) {
                start += LINK.size();
                std::string id(src.substr(start, 7));
                TRACE8("IMDbProgress::extractSearch (std::map&, const std::string&) - ID " << id);

                // Continue to end of href and extract the name from there
                start = src.find(">", start + 7);
                if (start != std::string::npos) {
                    std::string::size_type endLink(src.find("</a>", ++start));
                    if (endLink != std::string::npos) {
                        std::string name(src, start, endLink - start);
                        start = endLink + 4;
                        endLink = src.find(" <", start);
                        if (endLink != std::string::npos)
                            name += std::string(src, start, endLink - start);
                        TRACE8("IMDbProgress::extractSearch (std::map&, const std::string&) - Name " << name);
                        YGP::convertHTMLUnicode2UTF8(name);
                        target.emplace_back(id, name);
                        start = endLink + 1;
                        continue;
                    }
                }
            }
        }
        return;
    }
}

//-----------------------------------------------------------------------------
/// Restarts loading a film
/// \param idFilm (New) identification of a film
//-----------------------------------------------------------------------------
void IMDbProgress::reStart(const std::string& idFilm) {
    stop();
    start(idFilm);
}
