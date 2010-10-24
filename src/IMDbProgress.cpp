//PROJECT     : CDManager
//SUBSYSTEM   : Films
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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>
#include <YGP/ANumeric.h>

#include "IMDbProgress.h"

static const char* HOST ("www.imdb.com");
static const char* PORT ("http");
static const char NOPOSTER[] = "title_addposter.jpg";

static const char HTTP[] = "http://";
static const char LINK[] = "a href=\"/title/tt";

struct ConnectInfo {
   boost::asio::io_service svcIO;
   boost::asio::ip::tcp::resolver resolver;
   boost::asio::ip::tcp::socket sockIO;
   boost::asio::streambuf buffer;

   std::string host;
   std::string path;
   Glib::ustring response;

   /// Constructor
   ConnectInfo (const Glib::ustring& url);
   ~ConnectInfo () {
      sockIO.close ();
      svcIO.stop ();
   }

 private:
   static bool isNumber (const Glib::ustring& nr);
};

//-----------------------------------------------------------------------------
/// Checks if the passed text is a number
/// \param nr Number to inspect
/// \returns bool True, if the passed text is a number
//-----------------------------------------------------------------------------
bool ConnectInfo::isNumber (const Glib::ustring& nr) {
   for (unsigned int i (0); i < nr.length (); ++i)
      if (!isdigit (nr[i]))
	 return false;
   return true;
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param id String identifying the film to load; can be an URL, an IMDb ID or a film name
//-----------------------------------------------------------------------------
ConnectInfo::ConnectInfo (const Glib::ustring& id)
   : svcIO (), resolver (svcIO), sockIO (svcIO), buffer (), host (HOST), path (), response () {
   Glib::ustring::size_type pos (Glib::ustring::npos);
   if (!id.compare (0, sizeof (HTTP) - 1, HTTP)
       && ((pos = id.find ('/', sizeof (HTTP))) != Glib::ustring::npos)) {
      // Starts with http:// and has a slash (/) separating host and path
      host = id.substr (sizeof (HTTP) - 1, pos - sizeof (HTTP) + 1);
      path = id.substr (pos);
   }
   else {
      // Doesn't seem to be an URL; so check for IMDb identifier or name
      if ((id.length () == 9) && !id.compare (0, 2, "tt") && isNumber (id.substr (2)))
	 path = "title/" + id + '/';
      else if (isNumber (id) > 0)
	 path = "title/tt" + Glib::ustring (7 - id.length (), '0') + id + '/';
      else {
	 path = id;
	 while ((pos = path.find (' ', pos + 1)) != Glib::ustring::npos)
	    path.replace (pos, 1, 1, '+');
	 path = "find?s=tt&q=" + path;
      }
   }
   TRACE1 ("ConnectInfo::ConnectInfo (const Glib::ustring&) - " << host << " - " << path)
}


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress () : Gtk::ProgressBar (), data (NULL), status (NONE) {
   TRACE5 ("IMDbProgress::IMDbProgress ()");
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param film ID of film to import
//-----------------------------------------------------------------------------
IMDbProgress::IMDbProgress (const Glib::ustring& film) : Gtk::ProgressBar (), data (NULL), status (NONE) {
   TRACE5 ("IMDbProgress::IMDbProgress (const Glib::ustring&) - " << film);
   start (film);
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
/// \param film Film to load; this can be either its name, its
///              number on IMDb.com or its whole URL
//-----------------------------------------------------------------------------
void IMDbProgress::start (const Glib::ustring& film) {
   TRACE1 ("IMDbProgress::start (const Glib::ustring&) - " << film);

   Check1 (!data); Check1 (status == NONE);
   status = TITLE;
   data = new ConnectInfo (film);
   set_text (_("Connecting to IMDb.com ..."));
   pulse ();

   connect ();
   conPoll = Glib::signal_timeout ().connect (mem_fun (*this, &IMDbProgress::poll), 50);

   conProgress = Glib::signal_timeout ().connect (mem_fun (*this, &IMDbProgress::indicateWait), 150);
}

//-----------------------------------------------------------------------------
/// Loads the specified icon
/// \param url Icon to load in format http://host/path/name
/// \note It is not save to call this method while handling the callback of a signal
//-----------------------------------------------------------------------------
void IMDbProgress::loadIcon (const std::string& url) {
   TRACE1 ("IMDbProgress::loadIcon (const std::string&) - " << url);
   Check1 (!data); Check1 (status == NONE);
   status = IMAGE;
   data = new ConnectInfo (url);
   set_text (_("Loading icon ..."));
   pulse ();

   connect();
   conPoll = Glib::signal_timeout ().connect (mem_fun (*this, &IMDbProgress::poll), 50);

   conProgress = Glib::signal_timeout ().connect (mem_fun (*this, &IMDbProgress::indicateWait), 150);
}

//-----------------------------------------------------------------------------
/// Stops the communication
/// \note It is not save to call this method while handling the callback of a signal
//-----------------------------------------------------------------------------
void IMDbProgress::stop () {
   TRACE3 ("IMDbProgress::stop ()");
   status = NONE;
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
   if (data->response.size ()) {
      Glib::ustring msg (_("Receiving from IMDb.com: %1 KB"));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric (data->response.size () >> 10).toString ());
      set_text (msg);
   }
   pulse ();
   return true;
}

//-----------------------------------------------------------------------------
/// Opens a connection to IMDb.com
//-----------------------------------------------------------------------------
void IMDbProgress::connect () {
   TRACE5 ("IMDbProgress::connect ()");
   Check2 (data);

   boost::asio::ip::tcp::resolver::query query (data->host, PORT);
   data->resolver.async_resolve (query, boost::bind (&IMDbProgress::resolved, this,
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
void IMDbProgress::connected (const boost::system::error_code& err,
			      boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE2 ("IMDbProgress::connected (boost::asio::streambuf*, boost::system::error_code&, iterator)");
   Check2 (data);

   // The connection was successful. Send the request.
   if (!err)
      sendRequest ();
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
/// \param film Identification of the film; can be its name or identity
//-----------------------------------------------------------------------------
void IMDbProgress::sendRequest () {
   Check1 (data);
   std::ostream request (&data->buffer);

   TRACE7 ("IMDbProgress::sendRequest (const Glib::ustring&) - " << data->path);
   request << "GET /" << data->path << " HTTP/1.0\r\nHost: " << data->host
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
      data->response.clear ();
      char buffer[256];
      do  {
	 response.read (buffer, sizeof(buffer));
	 data->response.append (buffer, response.gcount ());
      } while (response);

      boost::asio::async_read (data->sockIO, data->buffer, boost::asio::transfer_at_least (1),
			       boost::bind (&IMDbProgress::readContent, this,
					    boost::asio::placeholders::error));
   }
   else
      error (err.message ());
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
void IMDbProgress::readContent (const boost::system::error_code& err) {
   TRACE7 ("IMDbProgress::readContent (boost::system::error_code&)");
   Check2 (data);

   Glib::ustring msg;
   if (!err) {
      std::istream response (&data->buffer);
      char buffer[256];
      do  {
	 response.read (buffer, sizeof(buffer));
	 data->response.append (buffer, response.gcount ());
      } while (response);

      // Continue reading remaining data until EOF.
      boost::asio::async_read (data->sockIO, data->buffer,
			      boost::asio::transfer_at_least (1),
			       boost::bind (&IMDbProgress::readContent, this,
					    boost::asio::placeholders::error));
      return;
   }
   else if (err == boost::asio::error::eof) {
      if (status == TITLE) {
	 readFilm (msg);
	 if (msg.empty ())
	    return;
      }
      else {
	 readImage ();
	 return;
      }
   }
   else
      msg = err.message ();

   error (msg);
}


//-----------------------------------------------------------------------------
/// Reads the icon from the connection and emits a signal
//-----------------------------------------------------------------------------
void IMDbProgress::readImage () {
   disconnect ();
   sigIcon.emit (data->response);
}

//-----------------------------------------------------------------------------
/// Extracts information about a film from the read input and emits a signal
/// informing about the read data
/// \param msg String to write an error message into, if any
//-----------------------------------------------------------------------------
void IMDbProgress::readFilm (Glib::ustring& msg) {
   msg.clear ();
   std::string name (extract ("<head>", NULL, "<title>", "</title>"));
   TRACE4 ("IMDbProgress::readFilm (boost::system::error_code&) - Final: " << name << ": " << data->response.size ());

   if (name == "IMDb Title Search") {           // IMDb's search page found
      std::map<match, IMDbSearchEntries> films;
      unsigned int cFilms (0);

      const char* sections[] = { "<p><b>Popular Titles</b>", "<p><b>Titles (Exact Matches)</b>",
				 "<p><b>Titles (Partial Matches)</b>", "<p><b>Titles (Approx Matches)</b>" };
      for (unsigned int i(0); i < (sizeof (sections) / sizeof (sections[0])); ++i) {
	 extractSearch (films[(match)i], data->response, sections, i);
	 cFilms += films[(match)i].size ();
      }
      TRACE5 ("Films: " << cFilms);

      if (!cFilms)
	 msg = _("IMDb didn't find any matching films!");
      else if (cFilms == 1) {
	 for (unsigned int i(0); i < (sizeof (sections) / sizeof (sections[0])); ++i)
	    if (films[(match)i].size ())
	       Glib::signal_idle ().connect
		  (bind (mem_fun (*this, &IMDbProgress::reStart), films[(match)i].begin ()->url));
      }
      else {
	 disconnect ();
	 sigAmbiguous.emit (films);
      }
   }
   else {
      name.erase (name.length () - 7);
      std::string director (extract ("Director:", " href=\"/name/nm", ">", "</a>"));
      Glib::ustring genre (extract ("Genres:</h4>", "<a href=\"/genre/", "\">", "</a>"));
      std::string summary (extract ("<h2>Storyline</h2>", NULL, "<p>", "\n"));
      std::string image (extract ("img_primary", "<img", "src=\"", "\""));
      YGP::convertHTMLUnicode2UTF8 (director);
      YGP::convertHTMLUnicode2UTF8 (name);
      YGP::convertHTMLUnicode2UTF8 (summary);

      TRACE1 ("IMDbProgress::readFilm (boost::system::error_code&) - Director: " << director);
      TRACE1 ("IMDbProgress::readFilm (boost::system::error_code&) - Name: " << name);
      TRACE1 ("IMDbProgress::readFilm (boost::system::error_code&) - Genre: " << genre);
      TRACE1 ("IMDbProgress::readFilm (boost::system::error_code&) - Summary: " << summary);
      TRACE1 ("IMDbProgress::readFilm (boost::system::error_code&) - Icon: " << image);

      if (director.size () || name.size ()) {
	 if (image.length()
	     && !image.compare(image.length () - sizeof (NOPOSTER) + 1, sizeof (NOPOSTER) - 1, NOPOSTER))
	    image.clear ();
	 IMDbEntry entry (director, name, genre, summary, image);
	 disconnect ();
	 sigSuccess (entry);
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
Glib::ustring IMDbProgress::extract (const char* section, const char* subpart,
				     const char* before, const char* after) const {
   Check1 (section); Check1 (before); Check1 (after); Check2 (data);

   Glib::ustring::size_type i (data->response.find (section));
   if (i != std::string::npos)
      if (!subpart || ((i = data->response.find (subpart, i)) != std::string::npos))
	 if ((i = data->response.find (before, i)) != std::string::npos) {
	    i += strlen (before);

	    // Skip white-space at beginning
	    i = data->response.find_first_not_of (" \t\n\r", i);
	    std::string::size_type end (data->response.find (after, i));
	    return (end == std::string::npos) ? Glib::ustring () : data->response.substr (i, end - i);
	 }
   return Glib::ustring ();
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
void IMDbProgress::extractSearch (IMDbSearchEntries& target, const Glib::ustring& src,
				  const char** texts, unsigned int offset) {
   Glib::ustring::size_type start (src.find (texts[offset]));
   Glib::ustring::size_type end (src.find ("<p><b>", start + 10));
   TRACE1 ("IMDbProgress::extractSearch (std::map&, const Glib::ustring&) - Search from " << start << '-' << end);

   while (start < end) {
      // Align with lines of result
      start = src.find ("<tr>", start);
      if ((start != Glib::ustring::npos) && (start < end)) {
	 // The name of the film is in the 3rd column
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
		     target.push_back(IMDbSearchEntry (id, name));
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
/// Restarts loading a film
/// \param idFilm (New) identification of a film
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool IMDbProgress::reStart (const Glib::ustring& idFilm) {
   stop ();
   start (idFilm);
   return false;
}
