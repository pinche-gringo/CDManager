//PROJECT     : CDManager
//SUBSYSTEM   : Movies
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 04.04.2010
//COPYRIGHT   : Copyright (C) 2010

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

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>

#define CHECK 9
#define TRACELEVEL 8
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>
#include <YGP/ANumeric.h>

#include "IMDbProgress.h"

#ifdef LOCALTEST
static const char* HOST ("localhost");
#else
static const char* HOST ("www.imdb.com");
#endif
static const char* PORT ("http");

static const char SKIP[] = "http://www.imdb.com/";
static const char LINK[] = "a href=\"/title/tt";

struct ConnectInfo {
   boost::asio::io_service svcIO;
   boost::asio::ip::tcp::resolver::query query;
   boost::asio::ip::tcp::resolver resolver;
   boost::asio::ip::tcp::socket sockIO;
   boost::asio::streambuf buffer;

   Glib::ustring contentIMDb;
   Glib::ustring movie;

   /// Constructor
   ConnectInfo (const Glib::ustring& movie)
      : svcIO (), query (HOST, PORT), resolver (svcIO), sockIO (svcIO),
	buffer (), contentIMDb (), movie (movie) { }
   ~ConnectInfo () {
      sockIO.close ();
      svcIO.stop ();
   }
};

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress () : Gtk::ProgressBar (), data (NULL) {
   TRACE5 ("IMDbProgress::IMDbProgress ()");
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param movie ID of movie to import
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress (const Glib::ustring& movie) : Gtk::ProgressBar (), data (NULL) {
   TRACE5 ("IMDbProgress::IMDbProgress (const Glib::ustring&) - " << movie);
   start (movie);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
IMDbProgress::~IMDbProgress () {
   TRACE6 ("IMDbProgress::~IMDbProgress ()");
   stop ();
}


//-----------------------------------------------------------------------------
/// Starts the communication
/// \param movie Movie to load; this can be either its name, its
///              number on IMDb.com or its whole URL
//-----------------------------------------------------------------------------
void IMDbProgress::start (const Glib::ustring& movie) {
   TRACE1 ("IMDbProgress::start (const Glib::ustring&) - " << movie);

   Check1 (!data);
   delete data;
   data = new ConnectInfo (movie);
   set_text (_("Connecting to IMDb.com ..."));
   pulse ();

   connectToIMDb ();

   conPoll = Glib::signal_timeout ().connect
      (mem_fun (*this, &IMDbProgress::poll), 50);

   conProgress = Glib::signal_timeout ().connect
      (mem_fun (*this, &IMDbProgress::indicateWait), 150);
}

//-----------------------------------------------------------------------------
/// Stops the communication
/// \note It is not save to call this method while handling the callback of a signal
//-----------------------------------------------------------------------------
void IMDbProgress::stop () {
   TRACE3 ("IMDbProgress::stop ()");
   if (data) {
      disconnect ();

      delete data;
      data = NULL;
   }
}

//-----------------------------------------------------------------------------
/// Stops polling for events
//-----------------------------------------------------------------------------
void IMDbProgress::disconnect () {
   if (conPoll.connected ())
      conPoll.disconnect ();
   if (conProgress.connected ())
      conProgress.disconnect ();
}

//-----------------------------------------------------------------------------
/// Polls for available boost:asio-events
/// \returns bool Always true, indicating to continue with polling
//-----------------------------------------------------------------------------
bool IMDbProgress::poll () {
   TRACE9 ("IMDbProgress::poll ()");
   Check2 (data);

   data->svcIO.poll ();
   return true;
}

//-----------------------------------------------------------------------------
/// Updates the progress-bar while information is still loaded.
//-----------------------------------------------------------------------------
bool IMDbProgress::indicateWait () {
   Check2 (data);
   if (data->contentIMDb.size ()) {
      Glib::ustring msg (_("Receiving from IMDb.com: %1 KB"));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric (data->contentIMDb.size () >> 10).toString ());
      set_text (msg);
   }
   pulse ();
   return true;
}

//-----------------------------------------------------------------------------
/// Opens a connection to IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::connectToIMDb () {
   TRACE5 ("IMDbProgress::connectToIMDb ()");
   Check2 (data);

   data->resolver.async_resolve (data->query, boost::bind (&IMDbProgress::resolved, this,
							   boost::asio::placeholders::error,
							   boost::asio::placeholders::iterator));
   data->svcIO.poll ();
}

//-----------------------------------------------------------------------------
/// Callback after resolving the name of IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to available endpoints (in case of success)
//-----------------------------------------------------------------------------
void IMDbProgress::resolved (const boost::system::error_code& err,
			     boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE7 ("IMDbProgress::resolved (boost::system::error_code&, iterator)");
   Check2 (data);

   if (!err) {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      data->sockIO.async_connect
	 (*iEndpoints, boost::bind (&IMDbProgress::connected, this,
				    boost::asio::placeholders::error, iEndpoints));
   }
   else
      error (err.message ());
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
/// \param msg Message to display
//-----------------------------------------------------------------------------
void IMDbProgress::error (const Glib::ustring& msg) {
   disconnect ();
   sigError.emit (msg);
}

//-----------------------------------------------------------------------------
/// Callback after connecting to IMDb.com. Tries to load the movie
/// specified at construction-time/by the last start()-call according
/// to the following algorithm:
///   - If it starts with http://www.imdb.com/ use the string as is
///   - If it is "tt" followed by (exactly) 7 digits consider it an IMDb-ID
///   - If it is a number consider it an IMDb-ID
///   - Else perform a search within all titles
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to remaining endpoints
/// \note To search for a movie having a number as title (e.g. 1984) put it within quotes
//-----------------------------------------------------------------------------
void IMDbProgress::connected (const boost::system::error_code& err,
			      boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE2 ("IMDbProgress::connected (boost::asio::streambuf*, boost::system::error_code&, iterator)");
   Check2 (data);

   // The connection was successful. Send the request.
   if (!err)
      sendRequest (data->movie);
   else {
      // The connection failed. Try the next endpoint in the list.
      data->sockIO.close ();
      if (iEndpoints != boost::asio::ip::tcp::resolver::iterator ())
	 resolved (err, ++iEndpoints);
      else
	 resolved (boost::asio::error::host_not_found, iEndpoints);
   }
}

//-----------------------------------------------------------------------------
/// Sending the request about a title to IMDb.com
/// \param movie Identification of the movie; can be its name or identity
//-----------------------------------------------------------------------------
void IMDbProgress::sendRequest (const Glib::ustring& movie) {
   Check1 (data);
   std::ostream request (&data->buffer);
   Glib::ustring path (movie);

#ifdef LOCALTEST
   path = (path != "s") ? "index.html" : "search.html";
#else
   if (!path.compare (0, sizeof (SKIP) - 1, SKIP)) {
      // Starts with http://www.imdb.com/ -> Just skip this
      path = path.substr (sizeof (SKIP) - 1);
      if (path[path.length () - 1] != '/')
	 path += '/';
   }
   else if ((path.length () == 9) && !path.compare (0, 2, "tt") && isNumber (path.substr (2)))
      path = "title/" + path + '/';
   else if (isNumber (path) > 0)
      path = "title/tt" + Glib::ustring (7 - path.length (), '0') + path + '/';
   else {
      Glib::ustring::size_type pos (-1);
      while ((pos = path.find (' ', pos + 1)) != Glib::ustring::npos)
	 path.replace (pos, 1, 1, '+');
      path = "find?s=tt&q=" + path;
   }
#endif

   TRACE7 ("IMDbProgress::sendRequest (const Glib::ustring&) - " << path << '/' << path.length ());
   request << "GET /" << path << " HTTP/1.0\r\nHost: " << HOST
	   << "\r\nAccept: */*\r\nConnection: close\r\n\r\n";

   boost::asio::async_write (data->sockIO, data->buffer,
			     boost::bind (&IMDbProgress::requestWritten, this,
					  boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------
/// Callback after having sent the HTTP-request
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::requestWritten (const boost::system::error_code& err) {
   TRACE1 ("IMDbProgress::requestWritten (const boost::system::error_code&)");
   Check2 (data);

   if (!err) {
      // Read the response status line.
      boost::asio::async_read_until (data->sockIO, data->buffer, "\r\n",
				     boost::bind (&IMDbProgress::readStatus, this,
						  boost::asio::placeholders::error));
   }
   else
      error (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading the HTTP-status
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readStatus (const boost::system::error_code& err) {
   TRACE1 ("IMDbProgress::readStatus (boost::system::error_code&)");
   Check2 (data);

   if (!err) {
      // Check that response is OK.
      std::istream response (&data->buffer);
      std::string idHTTP;
      unsigned int nrStatus;
      std::string msgStatus;

      response >> idHTTP >> nrStatus;
      std::getline (response, msgStatus);

      if (!response || (idHTTP.substr (0, 5) != "HTTP/")) {
	 Glib::ustring msg (_("Invalid response: `%1'"));
	 msg.replace (msg.find ("%1"), 2, idHTTP);
	 error (msg);
	 return;
      }

      switch (nrStatus) {
      case 200:  // HTTP OK
	 break;

      case 302: {  // HTTP Moved temporarily
	 std::string url, loc;
	 char ch;

	 do {
	    response >> loc >> ch;
	    response.putback (ch);
	    std::getline (response, url);
	    TRACE8 ("IMDbProgress::readStatus (boost::system::error_code&) - Location: " << loc);
	 } while (response && (loc.substr (0, 9) != "Location:"));
	 if (response && url.length ()) {
	    // Strip trailing whitespaces
	    Glib::ustring::size_type end (url.length ());
	    while (isspace (url[--end]))
	       ;
	    url.replace (end, url.length () - 1, 0, '\0');
	    Glib::signal_idle ().connect
	       (bind (mem_fun (*this, &IMDbProgress::reStart), url));
	 }
	 else
	    error (_("HTTP status code 302 does not contain a location"));
	 return;
      }

      default: {  // Other error
	 Glib::ustring msg (_("IMDb.com returned status code %1 %2"));
	 msg.replace (msg.find ("%1"), 2, YGP::ANumeric (nrStatus).toString ());
	 msg.replace (msg.find ("%2"), 2, msgStatus);
	 error (msg);
	 return;
      }
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until (data->sockIO, data->buffer, "\r\n\r\n",
				     boost::bind (&IMDbProgress::readHeaders, this,
						  boost::asio::placeholders::error));
   }
   else
      error (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading the headers
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readHeaders (const boost::system::error_code& err) {
   TRACE4 ("IMDbProgress::readHeaders (boost::system::error_code&)");
   Check2 (data);

   if (!err) {
      // Skip the response headers.
      std::istream response (&data->buffer);
      std::string line;
      while (std::getline (response, line) && (line != "\r"))
	 ;

      // Read the remaining content
      data->contentIMDb.clear ();
      while (std::getline (response, line))
	 data->contentIMDb += line;
      TRACE8 ("IMDbProgress::readHeaders (boost::system::error_code&) - " << data->contentIMDb);

      boost::asio::async_read (data->sockIO, data->buffer, boost::asio::transfer_at_least (1),
			       boost::bind (&IMDbProgress::readContent, this,
					    boost::asio::placeholders::error));
   }
   else
      error (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading (a part of) the content. The parsed content
/// is analysed if it contains a matching movie or IMDb's search page.
///
/// Depending on the content either the movie-information is extracted
/// and listeners are informed about the extracted data or the most
/// likely results of the search are extracted and the listeners are
/// informed about them.
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void IMDbProgress::readContent (const boost::system::error_code& err) {
   TRACE9 ("IMDbProgress::readContent (boost::system::error_code&)");
   Check2 (data);

   Glib::ustring msg;
   if (!err) {
      std::string line;
      std::istream response (&data->buffer);
      while (std::getline (response, line))
	 data->contentIMDb += line;

      // Continue reading remaining data until EOF.
      boost::asio::async_read (data->sockIO, data->buffer,
			      boost::asio::transfer_at_least (1),
			       boost::bind (&IMDbProgress::readContent, this,
					    boost::asio::placeholders::error));
      return;
   }
   else if (err == boost::asio::error::eof) {
      std::string name (extract ("<head>", NULL, "<title>", "</title>"));
      TRACE4 ("IMDbProgress::readContent (boost::system::error_code&) - Final: " << name << ": "
	      << data->contentIMDb.size ());

      if (name == "IMDb Title Search") {           // IMDb's search page found
	 std::map<match, IMDbEntries> movies;
	 unsigned int cMovies (0);

	 const char* sections[] = { "<p><b>Popular Titles</b>", "<p><b>Titles (Exact Matches)</b>",
				    "<p><b>Titles (Partial Matches)</b>" };
	 for (unsigned int i(0); i < (sizeof (sections) / sizeof (sections[0])); ++i) {
	    extractSearch (movies[(match)i], data->contentIMDb, sections, i);
	    cMovies += movies[(match)i].size ();
	 }
	 TRACE5 ("Movies: " << cMovies);

	 if (!cMovies)
	    msg = _("IMDb didn't find any matching movies!");
	 else if (cMovies == 1) {
	    for (unsigned int i(0); i < (sizeof (sections) / sizeof (sections[0])); ++i)
	       if (movies[(match)i].size ()) {
		  Glib::signal_idle ().connect
		     (bind (mem_fun (*this, &IMDbProgress::reStart), movies[(match)i].begin ()->url));
		  return;
	       }
	    Check (0);
	 }
	 else {
	    disconnect ();
	    sigAmbiguous.emit (movies);
	    return;
	 }
      }
      else {
	 std::string director (extract (">Director", "<a href=\"/name", "/';\">", "</a>"));
	 Glib::ustring genre (extract (">Genre:<", "<a href=\"/Sections/Genres/", "/\">", "</a>"));
	 YGP::convertHTMLUnicode2UTF8 (director);
	 YGP::convertHTMLUnicode2UTF8 (name);

	 TRACE1 ("IMDbProgress::readContent (boost::system::error_code&) - Director: " << director);
	 TRACE1 ("IMDbProgress::readContent (boost::system::error_code&) - Name: " << name);
	 TRACE1 ("IMDbProgress::readContent (boost::system::error_code&) - Genre: " << genre);

	 if (director.size () || name.size () || genre.size ()) {
	    disconnect ();
	    sigSuccess (director, name, genre);
	    return;
	 }
	 else
	    msg = _("Couldn't extract movie-information from IMDb! Maybe the site was redesigned ...");
      }
   }
   else
      msg = err.message ();

   error (msg);
}

//-----------------------------------------------------------------------------
/// Extracts a substring out of (a previously parsed) contentIMDb
/// \param section Section in contentIMDb which is followed by the searched text
/// \param subpart Text leading to the searched text
/// \param before Text immediately before the searched text
/// \param after Text immediately after the searched text
//-----------------------------------------------------------------------------
Glib::ustring IMDbProgress::extract (const char* section, const char* subpart,
				     const char* before, const char* after) const {
   Check1 (section); Check1 (before); Check1 (after); Check2 (data);

   Glib::ustring::size_type i (data->contentIMDb.find (section));
   if (i == std::string::npos)
      return Glib::ustring ();

   if (subpart)
      if ((i = data->contentIMDb.find (subpart, i)) == std::string::npos)
	 return Glib::ustring ();

   i = data->contentIMDb.find (before, i);
   if (i == std::string::npos)
      return Glib::ustring ();

   i += strlen (before);
   std::string::size_type end (data->contentIMDb.find (after, i));
   return (end == std::string::npos) ? Glib::ustring () : data->contentIMDb.substr (i, end - i);
}

//-----------------------------------------------------------------------------
/// Checks if the passed text is a number
/// \param nr Number to inspect
/// \returns bool True, if the passed text is a number
//-----------------------------------------------------------------------------
bool IMDbProgress::isNumber (const Glib::ustring& nr) {
   for (unsigned int i (0); i < nr.length (); ++i)
      if (!isdigit (nr[i]))
	 return false;
   return true;
}

//-----------------------------------------------------------------------------
/// Tries to extract IMDb-search results from the passed text. Found entries are
/// added to the passed map (with the ID as key and the name as value).
/// \param target Vector where the found entries are written to
/// \param src String to revise (HTML page read from IMDb)
/// \param text Array of texts for section heading the entried
/// \param offset Offset of text to search for
/// \returns bool True, if the passed text is a number
//-----------------------------------------------------------------------------
void IMDbProgress::extractSearch (IMDbEntries& target, const Glib::ustring& src,
				  const char** texts, unsigned int offset) {
   Glib::ustring::size_type start (src.find (texts[offset]));
   Glib::ustring::size_type end (src.find ("<p><b>", start + 10));
   TRACE1 ("IMDbProgress::extractSearch (std::map&, const Glib::ustring&) - Search from " << start << '-' << end);

   while (start < end) {
      // Align with lines of result
      start = src.find ("<tr>", start);
      if ((start != Glib::ustring::npos) && (start < end)) {
	 // The name of the movie is in the 3rd column
	 for (unsigned int i (0); i < 2; ++i) {
	    start = src.find ("<td", start + 3);
	    if (start == Glib::ustring::npos)
	       return;
	 }

	 // Extract the 7 digit long ID from the contained link
	 start = src.find (LINK, start);
	 if (start != Glib::ustring::npos) {
	    start += sizeof (LINK) - 1;
	    Glib::ustring id (src.substr (start, 7));

	    // Continue to end of href and extract the name from there
	    start = src.find ("\">", start + 7);
	    if (start != Glib::ustring::npos) {
	       start += 2;

	       Glib::ustring::size_type endLink (src.find ("</a>", start));
	       Glib::ustring::size_type posReplace (endLink - start);
	       if (endLink != Glib::ustring::npos) {
		  endLink = src.find (")", endLink + 4);
		  if (endLink != Glib::ustring::npos) {
		     std::string name (src, start, ++endLink - start);
		     name.replace (posReplace, 4, 0, '\0');
		     YGP::convertHTMLUnicode2UTF8 (name);
		     target.push_back(IMDbEntry (id, name));
		     start = endLink + 1;
		     continue;
		  }
	       }
	    }
	 }
      }
      return;
   }
}

//-----------------------------------------------------------------------------
/// Restarts loading a movie
/// \param idMovie (New) identification of a movie
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool IMDbProgress::reStart (const Glib::ustring& idMovie) {
   stop ();
   start (idMovie);
   return false;
}
