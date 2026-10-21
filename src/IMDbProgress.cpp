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

} // namespace

struct IMDbProgress::ConnectInfo {
    SoupSession* session {soup_session_new_with_options("user-agent", PACKAGE "/" VERSION, "timeout", 30U, nullptr)};
    SoupMessage* message {nullptr};
    GCancellable* cancel {g_cancellable_new()};

    IMDbParser::SearchTerm search; ///< Searched film (if searching by name)

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
    if (const std::string id(IMDbParser::getIMDbID(identifier)); !id.empty()) {
        status = TITLE;
        sendQuery(QUERY_TITLE, "id", id);
    }
    else {
        data->search = IMDbParser::getSearchTerm(identifier);
        status = SEARCH;
        sendQuery(QUERY_SEARCH, "term", data->search.term);
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
    if (const Glib::ustring msg(IMDbParser::getError(json)); !msg.empty()) {
        error(Glib::ustring::compose(_("IMDb.com reported an error: %1"), msg));
        return;
    }

    if (status == SEARCH)
        readSearch(json);
    else
        readFilm(json);
}

//-----------------------------------------------------------------------------
/// Extracts the results of a search; if it contains only one film or only one
/// film matches exactly, this is loaded, else the listeners are informed about
/// the found entries
/// \param response Response of IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::readSearch(const boost::json::value& response) {
    contract_assert(data);

    const IMDbParser::SearchResult result(IMDbParser::readSearch(response, data->search));
    if (result.films.at(IMDbParser::POPULAR).empty())
        error(_("IMDb didn't find any matching films!"));
    else if (!result.id.empty())
        Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*this, &IMDbProgress::reStart), result.id));
    else {
        disconnect();
        sigAmbiguous.emit(result.films);
    }
}

//-----------------------------------------------------------------------------
/// Extracts information about a film from the response and emits a signal
/// informing about the read data
/// \param response Response of IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::readFilm(const boost::json::value& response) {
    const IMDbEntry entry(IMDbParser::readFilm(response));
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
