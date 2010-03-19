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

#include <boost/bind.hpp>

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "ImportIMDb.h"


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb ()
   : XGP::XDialog (XGP::XDialog::NONE), sigLoaded (), client (3, 2), txtID (),
     status (QUERY), svcIO (), sockIO (svcIO) {
   set_title (_("Import from IMDb.com"));

   client.show ();

   Gtk::Label* lbl (new Gtk::Label (_("_Number or URL of the _movie:"), true));
   lbl->set_mnemonic_widget (txtID);
   client.attach (*manage (lbl), 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
   client.attach (txtID, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5, 5);

   get_vbox ()->pack_start (client, false, false, 5);

   txtID.signal_changed ().connect (mem_fun (*this, &ImportFromIMDb::inputChanged));

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
   ok->set_sensitive (txtID.get_text_length ());
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a button in the dialog
//-----------------------------------------------------------------------------
void ImportFromIMDb::okEvent () {
   TRACE1 ("ImportFromIMDb::okEvent () - " << status);

   if (status == QUERY) {
      status = LOADING;
      ok->set_sensitive (false);
      Gtk::ProgressBar* progress (new Gtk::ProgressBar);
      progress->pulse ();
      progress->show ();
      client.attach (*manage (progress), 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND,
		     Gtk::FILL | Gtk::EXPAND, 5, 5);

      Glib::signal_idle ().connect
	 (bind_return (mem_fun (*this, &ImportFromIMDb::connectToIMDb), false));

      Glib::signal_timeout ().connect
	 (bind (mem_fun (*this, &ImportFromIMDb::indicateWait), progress), 150);
   }
   else {
      Glib::ustring a;
      sigLoaded.emit (a, a);
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
   client.remove (*progress);
   return false;
}

//-----------------------------------------------------------------------------
/// Opens a connection to IMDb.com
//-----------------------------------------------------------------------------
void ImportFromIMDb::connectToIMDb () {
   TRACE5 ("ImportFromIMDb::connectToIMDb ()");
   boost::asio::ip::tcp::resolver::query query ("localhost", "http");
   boost::asio::ip::tcp::resolver resolver (svcIO);

   resolver.async_resolve
      (query, boost::bind (&ImportFromIMDb::resolved, this,
			   boost::asio::placeholders::error,
			   boost::asio::placeholders::iterator));
}

//-----------------------------------------------------------------------------
/// Callback after resolving the name of IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to available endpoints (in case of success)
//-----------------------------------------------------------------------------
void ImportFromIMDb::resolved (const boost::system::error_code& err,
			       boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE7 ("ImportFromIMDb::resolved (error_code&, iterator)");

   if (!err) {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      sockIO.async_connect
	 (*iEndpoints, boost::bind (&ImportFromIMDb::resolved, this,
				    boost::asio::placeholders::error, iEndpoints));
   }
   else {
      status = QUERY;
      Gtk::MessageDialog dlg (err.message (), Gtk::MESSAGE_ERROR);
      dlg.run ();
    }

}

//-----------------------------------------------------------------------------
/// Callback after connecting to IMDb.com
/// \param err Error-information (in case of error)
/// \param iEndpoints Iterator to remaining endpoints
//-----------------------------------------------------------------------------
void ImportFromIMDb::connected (const boost::system::error_code& err,
			       boost::asio::ip::tcp::resolver::iterator iEndpoints) {
   TRACE2 ("ImportFromIMDb::connected (error_code&, iterator)");

   if (!err) {
      // The connection was successful. Send the request.
      // boost::asio::async_write(socket_, request_,
      //			       boost::bind(&client::handle_write_request, this,
      //					   boost::asio::placeholders::error));
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
