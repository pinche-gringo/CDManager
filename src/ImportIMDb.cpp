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

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "IMDbProgress.h"

#include "ImportIMDb.h"


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb ()
   : XGP::XDialog (XGP::XDialog::NONE), sigLoaded (), client (new Gtk::Table (5, 2)),
     txtID (new Gtk::Entry), lblDirector (NULL), lblMovie (NULL), lblGenre (NULL),
     contentIMDb (), status (QUERY) {
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

      IMDbProgress* progress (new IMDbProgress (txtID->get_text ()));
      progress->sigError.connect (mem_fun (*this, &ImportFromIMDb::showError));
      progress->sigSuccess.connect (bind (mem_fun (*this, &ImportFromIMDb::showData), progress));
      progress->show ();
      client->attach (*progress, 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND,
		     Gtk::FILL | Gtk::EXPAND, 5, 5);
   }
   else if (status == CONFIRM) {
      Check3 (lblDirector); Check3 (lblMovie); Check3 (lblGenre);
      if (sigLoaded.emit (lblDirector->get_text (), lblMovie->get_text (), lblGenre->get_text ()))
	 response (Gtk::RESPONSE_OK);
   }
}

//-----------------------------------------------------------------------------
/// Updates the dialog with the data read
/// \param director Director of the parsed movie
/// \param name Name of the parsed movie
/// \param genre Genre of the parsed movie
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showData (const Glib::ustring& director, const Glib::ustring& name,
			       const Glib::ustring& genre, IMDbProgress* progress) {
   Check1 (progress); Check1 (client);
   progress->hide ();
   client->remove (*progress);
   delete progress;

   Gtk::Label* lbl (new Gtk::Label (_("Director:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER));
   lbl->show ();
   client->attach (*manage (lbl), 0, 1, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   lblDirector = new Gtk::Label (director, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
   lblDirector->show ();
   client->attach (*manage (lblDirector), 1, 2, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   lbl = new Gtk::Label (_("Movie:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
   lbl->show ();
   client->attach (*manage (lbl), 0, 1, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   lblMovie = new Gtk::Label (name, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
   lblMovie->show ();
   client->attach (*manage (lblMovie), 1, 2, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   lbl = new Gtk::Label (_("Genre:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
   lbl->show ();
   client->attach (*manage (lbl), 0, 1, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   lblGenre = new Gtk::Label (genre, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
   lblGenre->show ();
   client->attach (*manage (lblGenre), 1, 2, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   ok->set_label (Gtk::Stock::OK.id);
   ok->set_sensitive ();

   status = CONFIRM;
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
/// \param msg Message to display
//-----------------------------------------------------------------------------
void ImportFromIMDb::showError (const Glib::ustring& msg) {
   status = QUERY;
   inputChanged ();

   Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
   dlg.run ();
}
