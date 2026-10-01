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

#include <algorithm>
#include <array>
#include <string_view>

#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>

#include <glibmm/main.h>

#include <libsoup/soup.h>

#include <YGP/ANumeric.h>
#include <YGP/Trace.h>

#include "IMDbProgress.h"

namespace {

constexpr const char* GRAPHQL("https://caching.graphql.imdb.com/");
/// IMDb rejects GraphQL-requests not identifying the client
constexpr const char* CLIENT_NAME("imdb-web-next");
/// Image modifier making IMDb deliver posters with a height of 256 pixels
constexpr std::string_view POSTER_SIZE("SY256_");
constexpr std::string_view POSTER_MODIFIERS("._V1_");

constexpr std::string_view QUERY_TITLE(
    "query Title($id: ID!) { title(id: $id) {"
    " titleText { text } releaseYear { year } primaryImage { url } genres { genres { text } }"
    " plot { plotText { plainText } }"
    " summaries: plots(first: 1, filter: {type: SUMMARY}) { edges { node { plotText { plainText } } } }"
    " directors: credits(first: 1, filter: {categories: [\"director\"]}) { edges { node { name { nameText { text } } } } }"
    " cast: credits(first: 10, filter: {categories: [\"actor\", \"actress\"]}) { edges { node { name { nameText { text } } } } }"
    " } }");
constexpr std::string_view QUERY_SEARCH(
    "query Search($term: String!) { mainSearch(first: 50, options: {searchTerm: $term, type: TITLE}) {"
    " edges { node { entity { ... on Title { id titleText { text } originalTitleText { text } releaseYear { year }"
    " titleType { id text } } } } }"
    " } }");
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
/// Adds headers to the request, so IMDb returns titles and plots in the language of the user
/// \param headers Headers of the request
//-----------------------------------------------------------------------------
void addLanguage(SoupMessageHeaders* headers) {
    // Language names are like de_AT.UTF-8 or en_GB@euro; IMDb expects de-AT and AT
    std::string_view locale(g_get_language_names()[0]);
    locale = locale.substr(0, locale.find_first_of(".@"));
    if (const auto pos(locale.find('_')); (pos != std::string_view::npos) && (pos + 1 < locale.size())) {
        const std::string country(locale.substr(pos + 1));
        const std::string language(std::string(locale.substr(0, pos)) + '-' + country);
        TRACE5("addLanguage (SoupMessageHeaders*) - " << language);
        soup_message_headers_append(headers, "x-imdb-user-language", language.c_str());
        soup_message_headers_append(headers, "x-imdb-user-country", country.c_str());
    }
}

//-----------------------------------------------------------------------------
/// Checks if the passed character is a digit
/// \param ch Character to check
/// \returns bool True, if ch is a digit
//-----------------------------------------------------------------------------
bool isDigit(char ch) { return (ch >= '0') && (ch <= '9'); }

} // namespace

struct IMDbProgress::ConnectInfo {
    SoupSession* session {soup_session_new_with_options("user-agent", PACKAGE "/" VERSION, "timeout", 30U, nullptr)};
    SoupMessage* message {nullptr};
    GCancellable* cancel {g_cancellable_new()};

    Glib::ustring searchTitle; ///< Searched title (case-folded), to detect exact matches
    long searchYear {0};       ///< Searched year (or 0, if not specified)

    ConnectInfo() = default;
    ~ConnectInfo() {
        // The callback of a still running request is called with G_IO_ERROR_CANCELLED
        g_cancellable_cancel(cancel);
        g_clear_object(&message);
        g_object_unref(cancel);
        g_object_unref(session);
    }

    ConnectInfo(const ConnectInfo&) = delete;
    ConnectInfo& operator=(const ConnectInfo&) = delete;
};

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
/// Extracts the IMDb ID out of the passed identification of a film:
///   - If it is an URL containing /title/tt<number>, the tt<number> part
///   - If it is "tt" followed by digits consider it an IMDb-ID
///   - If it is a number consider it an IMDb-ID (without the leading tt)
/// \param identifier Identification of a film
/// \returns std::string IMDb ID (tt followed by at least 7 digits) or an empty string, if identifier
///          is not an ID (but e.g. a name of a film)
//-----------------------------------------------------------------------------
std::string IMDbProgress::getIMDbID(const Glib::ustring& identifier) {
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
std::string IMDbProgress::getSmallPoster(const std::string& url) {
    const auto start(url.rfind(POSTER_MODIFIERS));
    const auto ext(url.rfind('.'));
    if ((start == std::string::npos) || (ext < start + POSTER_MODIFIERS.size()))
        return url;
    return url.substr(0, start + POSTER_MODIFIERS.size()) + std::string(POSTER_SIZE) + url.substr(ext);
}

//-----------------------------------------------------------------------------
/// Starts the communication
/// \param identifier Film to load; this can be either its name, its
///                   number on IMDb.com or its whole URL
/// \param isImage Flag if identifier specifies (the URL of) an image
/// \note To search for a film having a number as title (e.g. 1984) put it within quotes
//-----------------------------------------------------------------------------
void IMDbProgress::start(const Glib::ustring& identifier, bool isImage) {
    TRACE1("IMDbProgress::start (const Glib::ustring&, bool) - " << identifier);

    contract_assert(!data);
    contract_assert(status == NONE);
    data = std::make_unique<ConnectInfo>();
    conProgress = Glib::signal_timeout().connect(sigc::mem_fun(*this, &IMDbProgress::indicateWait), 150);
    pulse();

    if (isImage) {
        status = IMAGE;
        set_text(_("Loading poster ..."));
        data->message = soup_message_new("GET", identifier.c_str());
        if (data->message)
            sendRequest();
        else
            Glib::signal_idle().connect_once(sigc::bind(
                sigc::mem_fun(*this, &IMDbProgress::error), Glib::ustring::compose(_("Invalid URL: `%1'"), identifier)));
        return;
    }

    set_text(_("Connecting to IMDb.com ..."));
    if (const std::string id(getIMDbID(identifier)); !id.empty()) {
        status = TITLE;
        sendQuery(QUERY_TITLE, "id", id);
    }
    else {
        Glib::ustring term(identifier);
        if ((term.size() > 2) && term.raw().starts_with('"') && term.raw().ends_with('"'))
            term = term.substr(1, term.size() - 2);
        else if (const std::string& raw(term.raw()); (raw.size() > 7) && raw.ends_with(')') && raw.substr(raw.size() - 7, 2) == " ("
                 && std::ranges::all_of(raw.substr(raw.size() - 5, 4), isDigit)) {
            // Search only for the title (without the year "(2026)"); the year is used to find an exact match
            data->searchYear = std::stol(raw.substr(raw.size() - 5, 4));
            term = raw.substr(0, raw.size() - 7);
        }
        data->searchTitle = term.casefold();
        status = SEARCH;
        sendQuery(QUERY_SEARCH, "term", term);
    }
}

//-----------------------------------------------------------------------------
/// Stops the communication
/// \note It is not save to call this method while handling the callback of a signal
//-----------------------------------------------------------------------------
void IMDbProgress::stop() {
    TRACE3("IMDbProgress::stop ()");
    status = NONE;
    disconnect();
    data.reset();
}

//-----------------------------------------------------------------------------
/// Stops indicating the progress
//-----------------------------------------------------------------------------
void IMDbProgress::disconnect() {
    if (conProgress.connected())
        conProgress.disconnect();
    contract_assert(!conProgress.connected());
}

//-----------------------------------------------------------------------------
/// Updates the progress-bar while information is still loaded.
/// \returns bool Always true, indicating to continue with updating
//-----------------------------------------------------------------------------
bool IMDbProgress::indicateWait() {
    pulse();
    return true;
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
/// Sends a query to the GraphQL-API of IMDb.com
/// \param query GraphQL query to send
/// \param variable Name of the (only) variable of the query
/// \param value Value of the variable
//-----------------------------------------------------------------------------
void IMDbProgress::sendQuery(std::string_view query, std::string_view variable, const Glib::ustring& value) {
    contract_assert(data);

    boost::json::object request;
    request["query"] = query;
    request["variables"] = boost::json::object {{variable, value.raw()}};
    const std::string body(boost::json::serialize(request));
    TRACE7("IMDbProgress::sendQuery (2x std::string_view, const Glib::ustring&) - " << body);

    data->message = soup_message_new("POST", GRAPHQL);
    GBytes* bytes(g_bytes_new(body.data(), body.size()));
    soup_message_set_request_body_from_bytes(data->message, "application/json", bytes);
    g_bytes_unref(bytes);

    SoupMessageHeaders* headers(soup_message_get_request_headers(data->message));
    soup_message_headers_append(headers, "x-imdb-client-name", CLIENT_NAME);
    addLanguage(headers);
    sendRequest();
}

//-----------------------------------------------------------------------------
/// Sends the prepared request asynchronously; received() is called with the response
//-----------------------------------------------------------------------------
void IMDbProgress::sendRequest() {
    contract_assert(data);
    contract_assert(data->message);

    soup_session_send_and_read_async(
        data->session, data->message, G_PRIORITY_DEFAULT, data->cancel,
        [](GObject* source, GAsyncResult* result, gpointer self) {
            GError* err(nullptr);
            GBytes* body(soup_session_send_and_read_finish(SOUP_SESSION(source), result, &err));
            if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                // The loading was stopped; so self might already be destroyed
                g_error_free(err);
                return;
            }

            auto* progress(static_cast<IMDbProgress*>(self));
            if (err) {
                const Glib::ustring msg(err->message);
                g_error_free(err);
                progress->error(msg);
                return;
            }

            gsize size(0);
            const auto* bytes(static_cast<const char*>(g_bytes_get_data(body, &size)));
            const std::string response(bytes ? std::string(bytes, size) : std::string());
            g_bytes_unref(body);
            progress->received(response);
        },
        this);
}

//-----------------------------------------------------------------------------
/// Callback after receiving the response to a request.
///
/// Depending on the request either the poster is passed to the listeners,
/// the film-information is extracted and listeners are informed about the
/// extracted data or the results of the search are extracted and the
/// listeners are informed about them.
/// \param response Received data
//-----------------------------------------------------------------------------
void IMDbProgress::received(const std::string& response) {
    contract_assert(data);
    contract_assert(data->message);

    const unsigned int httpStatus(soup_message_get_status(data->message));
    TRACE1("IMDbProgress::received (const std::string&) - Status " << httpStatus << "; " << response.size() << " bytes");
    if (httpStatus != SOUP_STATUS_OK) {
        error(Glib::ustring::compose(_("IMDb.com returned status code %1 %2"),
                                     Glib::ustring(YGP::ANumeric(httpStatus).toString()),
                                     Glib::ustring(soup_message_get_reason_phrase(data->message))));
        return;
    }

    if (status == IMAGE) {
        disconnect();
        sigIcon.emit(response);
        return;
    }

    boost::system::error_code err;
    const boost::json::value json(boost::json::parse(response, err));
    if (err) {
        error(Glib::ustring::compose(_("Invalid response from IMDb.com: %1"), Glib::ustring(err.message())));
        return;
    }
    if (const boost::json::array* errors(getArray(json, "/errors")); errors && !errors->empty()) {
        error(Glib::ustring::compose(_("IMDb.com reported an error: %1"), getText(errors->front(), "/message")));
        return;
    }

    if (status == SEARCH)
        readSearch(json);
    else
        readFilm(json);
}

//-----------------------------------------------------------------------------
/// Checks if the passed film matches the searched title (and year) exactly
/// \param film JSON object describing the film
/// \returns bool True, if the (localised or original) title matches (ignoring the case)
//-----------------------------------------------------------------------------
bool IMDbProgress::isExactMatch(const boost::json::value& film) const {
    if (data->searchYear) {
        const boost::json::value* year(find(film, "/releaseYear/year"));
        if (!year || !year->is_int64() || (year->get_int64() != data->searchYear))
            return false;
    }
    return (getText(film, "/titleText/text").casefold() == data->searchTitle)
           || (getText(film, "/originalTitleText/text").casefold() == data->searchTitle);
}

//-----------------------------------------------------------------------------
/// Extracts the results of a search; if it contains only one film or only one
/// film matches exactly, this is loaded, else the listeners are informed about
/// the found entries
/// \param response Response of IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::readSearch(const boost::json::value& response) {
    IMDbMatchData films;
    IMDbSearchEntries& entries(films[POPULAR]);
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
                TRACE8("IMDbProgress::readSearch (const boost::json::value&) - " << id << ": " << name);
                entries.emplace_back(id, name);
                if (isExactMatch(*film)) {
                    exactMatches.push_back(id);
                    if (type == "movie")
                        exactMovies.push_back(id);
                }
            }
    TRACE5("IMDbProgress::readSearch (const boost::json::value&) - Films: " << entries.size() << "; exact: "
           << exactMatches.size() << '/' << exactMovies.size());

    // Load the film directly, if it is the only one found or the only one matching exactly
    // (with preference to feature films, as there are often shorts or videos with the same title)
    std::string id;
    if (entries.size() == 1)
        id = entries.front().url;
    else if (exactMatches.size() == 1)
        id = exactMatches.front();
    else if (exactMovies.size() == 1)
        id = exactMovies.front();

    if (entries.empty())
        error(_("IMDb didn't find any matching films!"));
    else if (!id.empty())
        Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*this, &IMDbProgress::reStart), id));
    else {
        disconnect();
        sigAmbiguous.emit(films);
    }
}

//-----------------------------------------------------------------------------
/// Extracts information about a film from the response and emits a signal
/// informing about the read data
/// \param response Response of IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::readFilm(const boost::json::value& response) {
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

    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Director: " << entry.director);
    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Name: " << entry.title);
    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Genre: " << entry.genre);
    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Summary: " << entry.summary);
    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Icon: " << entry.image);
    TRACE1("IMDbProgress::readFilm (const boost::json::value&) - Actors: " << entry.actors.size());

    if (entry.title.empty()) {
        error(_("IMDb.com doesn't know the requested film!"));
        return;
    }
    disconnect();
    sigSuccess.emit(entry);
}

//-----------------------------------------------------------------------------
/// Restarts loading a film
/// \param idFilm (New) identification of a film
//-----------------------------------------------------------------------------
void IMDbProgress::reStart(const std::string& idFilm) {
    stop();
    start(idFilm);
}
