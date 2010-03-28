//$Id$

//PROJECT     : CDManager
//SUBSYSTEM   : Movies
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 19.03.2010
//COPYRIGHT   : Copyright (C) 2010

// This file is part of CDManager.
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

#include <istream>
#include <ostream>

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 8
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>

#include "ImportIMDb.h"


static const char* HOST ("localhost");
static const char* PORT ("http");


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb ()
   : XGP::XDialog (XGP::XDialog::NONE), sigLoaded (), client (new Gtk::Table (5, 2)),
     txtID (new Gtk::Entry), lblDirector (NULL), lblMovie (NULL), lblGenre (NULL),
     contentIMDb (), status (QUERY), svcIO (), sockIO (svcIO) {
   set_title (_("Import from IMDb.com"));

   client->show ();

   Gtk::Label* lbl (new Gtk::Label (_("_Number or URL of the _movie:"), true));
   lbl->set_mnemonic_widget (*txtID);
   client->attach (*manage (lbl), 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
   client->attach (*txtID, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5, 5);

   get_vbox ()->pack_start (*client, false, false, 5);

   txtID->signal_changed ().connect (mem_fun (*this, &ImportFromIMDb::inputChanged));

   ok = new Gtk::Button (Gtk::Stock::GO_FORWARD);
   get_action_area ()->pack_start (*ok, false, false, 5);
   ok->set_flags (Gtk::CAN_DEFAULT);
   ok->grab_default ();
   ok->signal_clicked ().connect (mem_fun (*this, &ImportFromIMDb::okEvent));

   cancel = add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

   inputChanged ();
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ImportFromIMDb::~ImportFromIMDb () {
}


//-----------------------------------------------------------------------------
/// Callback, if one of the edit-fields is changed
//-----------------------------------------------------------------------------
void ImportFromIMDb::inputChanged () {
   Check3 (ok);
   ok->set_sensitive (txtID->get_text_length ());
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a button in the dialog
//-----------------------------------------------------------------------------
void ImportFromIMDb::okEvent () {
   TRACE1 ("ImportFromIMDb::okEvent () - " << status);

   if (status == QUERY) {
      status = LOADING;
      txtID->set_sensitive (false);
      ok->set_sensitive (false);
      Gtk::ProgressBar* progress (new Gtk::ProgressBar);
      progress->set_text (_("Connecting to IMDB.com ..."));
      progress->pulse ();
      progress->show ();
      client->attach (*manage (progress), 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND,
		     Gtk::FILL | Gtk::EXPAND, 5, 5);

      Glib::signal_idle ().connect
	 (bind_return (mem_fun (*this, &ImportFromIMDb::connectToIMDb), false));

      Glib::signal_timeout ().connect
	 (bind (mem_fun (*this, &ImportFromIMDb::indicateWait), progress), 150);
   }
   else if (status == CONFIRM) {
      Check3 (lblDirector); Check3 (lblMovie); Check3 (lblGenre);
      sigLoaded.emit (lblDirector->get_text (), lblMovie->get_text (), lblGenre->get_text ());
   }
}

//-----------------------------------------------------------------------------
/// Updates the progress-bar while information is still loaded.
///
/// After querying the information the progress-bar is removed
//-----------------------------------------------------------------------------
bool ImportFromIMDb::indicateWait (Gtk::ProgressBar* progress) {
   Check1 (progress);

   if (status == LOADING) {
      progress->pulse ();
      return true;
   }
   progress->hide ();
   client->remove (*progress);
   return false;
}

//-----------------------------------------------------------------------------
/// Opens a connection to IMDb.com
//-----------------------------------------------------------------------------
void ImportFromIMDb::connectToIMDb () {
   TRACE5 ("ImportFromIMDb::connectToIMDb ()");
   boost::asio::ip::tcp::resolver::query query (HOST, PORT);
   boost::asio::ip::tcp::resolver resolver (svcIO);

   resolver.async_resolve
      (query, boost::bind (&ImportFromIMDb::resolved, this,
			   boost::asio::placeholders::error,
			   boost::asio::placeholders::iterator));
   svcIO.run ();
}

//-----------------------------------------------------------------------------
/// Callback after resolving the name of IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to available endpoints (in case of success)
//-----------------------------------------------------------------------------
void ImportFromIMDb::resolved (const boost::system::error_code& err,
			       boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE7 ("ImportFromIMDb::resolved (boost::system::error_code&, iterator)");

   if (!err) {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      sockIO.async_connect
	 (*iEndpoints, boost::bind (&ImportFromIMDb::connected, this,
				    boost::asio::placeholders::error, iEndpoints));
   }
   else
      showError (err.message ());
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
//-----------------------------------------------------------------------------
void ImportFromIMDb::showError (const Glib::ustring& msg) {
   sockIO.close ();
   svcIO.stop ();
   status = QUERY;
   Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
   dlg.run ();

   inputChanged ();
   txtID->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Callback after connecting to IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to remaining endpoints
//-----------------------------------------------------------------------------
void ImportFromIMDb::connected (const boost::system::error_code& err,
			       boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE2 ("ImportFromIMDb::connected (boost::system::error_code&, iterator)");

   if (!err) {
      // The connection was successful. Send the request.
      std::ostream request (&buf);
      request << "GET /" << txtID->get_text () << " HTTP/1.0\r\nHost: " << HOST
	      << "\r\nAccept: */*\r\nConnection: close\r\n\r\n";

      boost::asio::async_write (sockIO, buf,
				boost::bind (&ImportFromIMDb::written, this,
					     boost::asio::placeholders::error));
   }
   else {
      // The connection failed. Try the next endpoint in the list.
      sockIO.close ();
      if (iEndpoints != boost::asio::ip::tcp::resolver::iterator ())
	 resolved (err, ++iEndpoints);
      else
	 resolved (boost::asio::error::host_not_found, iEndpoints);
   }
}

//-----------------------------------------------------------------------------
/// Callback after having sent the HTTP-request
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void ImportFromIMDb::written (const boost::system::error_code& err) {
   TRACE1 ("ImportFromIMDb::written (boost::system::error_code&");

   if (!err) {
      // Read the response status line.
      boost::asio::async_read_until (sockIO, buf, "\r\n",
				     boost::bind (&ImportFromIMDb::readStatus, this,
						  boost::asio::placeholders::error));
   }
   else
      showError (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading the HTTP-status
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void ImportFromIMDb::readStatus (const boost::system::error_code& err) {
   TRACE1 ("ImportFromIMDb::readStatus (boost::system::error_code&)");

   if (!err) {
      // Check that response is OK.
      std::istream response (&buf);
      std::string idHTTP;
      unsigned int nrStatus;
      std::string msgStatus;

      response >> idHTTP >> nrStatus;
      std::getline (response, msgStatus);

      if (!response || (idHTTP.substr (0, 5) != "HTTP/")) {
	 Glib::ustring msg (_("Invalid response: `%1'"));
	 msg.replace (msg.find ("%1"), 2, idHTTP);
	 showError (msg);
	 return;
      }

      if (nrStatus != 200) {
	 Glib::ustring msg (_("Response returned with status code %1"));
	 msg.replace (msg.find ("%1"), 2, YGP::ANumeric (nrStatus).toString ());
	 showError (msg);
	 return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until (sockIO, buf, "\r\n\r\n",
				     boost::bind (&ImportFromIMDb::readHeaders, this,
						  boost::asio::placeholders::error));
   }
   else
      showError (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading the headers
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void ImportFromIMDb::readHeaders (const boost::system::error_code& err) {
   TRACE1 ("ImportFromIMDb::readHeaders (boost::system::error_code&)");

   if (!err) {
      // Skip the response headers.
      std::istream response (&buf);
      std::string line;
      while (std::getline (response, line) && (line != "\r"))
	 ;

      // Read the remaining content
      contentIMDb.clear ();
      while (std::getline (response, line))
	 contentIMDb += line;
      TRACE8 ("ImportFromIMDb::readHeaders (boost::system::error_code&) - " << contentIMDb);

      boost::asio::async_read (sockIO, buf, boost::asio::transfer_at_least (1),
			       boost::bind (&ImportFromIMDb::readContent, this,
					    boost::asio::placeholders::error));
   }
   else
      showError (err.message ());
}

//-----------------------------------------------------------------------------
/// Callback after reading (a part of) the content
/// \param err Error-information (in case of error)
//-----------------------------------------------------------------------------
void ImportFromIMDb::readContent (const boost::system::error_code& err) {
   TRACE9 ("ImportFromIMDb::readContent (boost::system::error_code&)");

   if (!err) {
      std::string line;;
      std::istream response (&buf);
      while (std::getline (response, line))
	 contentIMDb += line;

      // Continue reading remaining data until EOF.
      boost::asio::async_read (sockIO, buf,
			      boost::asio::transfer_at_least (1),
			       boost::bind (&ImportFromIMDb::readContent, this,
					    boost::asio::placeholders::error));
   }
   else if (err == boost::asio::error::eof) {
      TRACE9 ("ImportFromIMDb::readContent (boost::system::error_code&) - Final: " << contentIMDb);
      status = CONFIRM;

      // Extract director
      Glib::ustring director (extract (">Director:<", "<a href=\"/name", "/';\">", "</a>"));
      Glib::ustring name (extract ("<head>", NULL, "<title>", "</title>"));
      Glib::ustring genre (extract (">Genre:<", "<a href=\"/Sections/Genres/", "/\">", "</a>"));

      TRACE1 ("ImportFromIMDb::readContent (boost::system::error_code&) - Director: " << director);
      TRACE1 ("ImportFromIMDb::readContent (boost::system::error_code&) - Name: " << name);
      TRACE1 ("ImportFromIMDb::readContent (boost::system::error_code&) - Genre: " << genre);

      Gtk::Label* lbl (new Gtk::Label (_("Director: "), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER));
      lbl->show ();
      client->attach (*manage (lbl), 0, 1, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
      lblDirector = new Gtk::Label (director, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
      lblDirector->show ();
      client->attach (*manage (lblDirector), 1, 2, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

      lbl = new Gtk::Label (_("Movie: "), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
      lbl->show ();
      client->attach (*manage (lbl), 0, 1, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
      lblMovie = new Gtk::Label (name, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
      lblMovie->show ();
      client->attach (*manage (lblMovie), 1, 2, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

      lbl = new Gtk::Label (_("Genre: "), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
      lbl->show ();
      client->attach (*manage (lbl), 0, 1, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
      lblGenre = new Gtk::Label (genre, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
      lblGenre->show ();
      client->attach (*manage (lblGenre), 1, 2, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

      ok->set_label (Gtk::Stock::OK.id);
      ok->set_sensitive ();
   }
   else
      showError (err.message ());
}

//-----------------------------------------------------------------------------
/// Extracts a substring out of (a previously parsed) contentIMDb
/// \param section Section in contentIMDb which is followed by the searched text
/// \param subpart Text leading to the searched text
/// \param before Text immediately before the searched text
/// \param after Text immediately after the searched text
//-----------------------------------------------------------------------------
Glib::ustring ImportFromIMDb::extract (const char* section, const char* subpart,
				     const char* before, const char* after) const {
   Check1 (section); Check1 (before); Check1 (after);

   Glib::ustring::size_type i (contentIMDb.find (section));
   if (i == std::string::npos)
      return Glib::ustring ();

   if (subpart)
      if ((i = contentIMDb.find (subpart, i)) == std::string::npos)
	 return Glib::ustring ();

   i = contentIMDb.find (before, i);
   if (i == std::string::npos)
      return Glib::ustring ();

   i += strlen (before);
   std::string::size_type end (contentIMDb.find (after, i));
   return (end == std::string::npos) ? Glib::ustring () : contentIMDb.substr (i, end - i);
}
