//$Id$

//PROJECT     : CDManager
//SUBSYSTEM   : Movies
//REFERENCES  :
//TODO        : - Remove progress-bar when canceling
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
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "IMDbProgress.h"

#include "ImportIMDb.h"


/**Class defining the columns for the list of results of the IMDb-search
 */
class MovieColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   MovieColumns () : Gtk::TreeModel::ColumnRecord () { add (id); add (name); }

   Gtk::TreeModelColumn<Glib::ustring> id;
   Gtk::TreeModelColumn<Glib::ustring> name;
};


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb ()
   : XGP::XDialog (XGP::XDialog::NONE), sigLoaded (), client (new Gtk::Table (5, 2)),
     txtID (new Gtk::Entry), lblDirector (NULL), lblMovie (NULL), lblGenre (NULL),
     contentIMDb (), status (QUERY) {
   set_title (_("Import from IMDb.com"));

   client->show ();

   Gtk::Label* lbl (new Gtk::Label (_("_Name, number or URL of the movie:"), true));
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
/// Callback, if one entry of the list box is (de)selected
/// \param list: List to examine
//-----------------------------------------------------------------------------
void ImportFromIMDb::rowSelected (Gtk::TreeView* list) {
   Check2 (ok); Check2 (list);
   ok->set_sensitive (list->get_selection ()->get_selected ());
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a button in the dialog
//-----------------------------------------------------------------------------
void ImportFromIMDb::okEvent () {
   TRACE1 ("ImportFromIMDb::okEvent () - " << status);

   switch (status) {
   case QUERY: {
      status = LOADING;
      txtID->set_sensitive (false);
      ok->set_sensitive (false);

      IMDbProgress* progress (new IMDbProgress (txtID->get_text ()));
      progress->sigError.connect (bind (mem_fun (*this, &ImportFromIMDb::showError), progress));
      progress->sigAmbiguous.connect (bind (mem_fun (*this, &ImportFromIMDb::showSearchResults), progress));
      progress->sigSuccess.connect (bind (mem_fun (*this, &ImportFromIMDb::showData), progress));
      progress->show ();
      client->attach (*manage (progress), 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND,
		     Gtk::FILL | Gtk::EXPAND, 5, 5);
      break;
   }

   case CHOOSING:
      // Do nothing; any action is performed by other handlers
      break;

   case CONFIRM:
      Check3 (lblDirector); Check3 (lblMovie); Check3 (lblGenre);
      if (sigLoaded.emit (lblDirector->get_text (), lblMovie->get_text (), lblGenre->get_text ()))
	 response (Gtk::RESPONSE_OK);

   default:
      Check (0);
   }
}

//-----------------------------------------------------------------------------
/// Removes the progressbar
/// \param client Client area from which remove the progressbar from
/// \param progress Progressbar to remove
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool ImportFromIMDb::removeProgressBar (Gtk::Table* client, IMDbProgress* progress) {
   TRACE9 ("ImportFromIMDb::removeProgressBar (Gtk::Table*, IMDbProgress*)");
   Check1 (progress); Check1 (client);
   progress->stop ();
   client->remove (*progress);
   return false;
}

//-----------------------------------------------------------------------------
/// Stops the loading of data of the progressbar
/// \param progress Progressbar to stop
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool ImportFromIMDb::stopLoading (IMDbProgress* progress) {
   TRACE9 ("ImportFromIMDb::stopLoading (IMDbProgress*)");
   Check1 (progress);
   progress->stop ();
   return false;
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
   TRACE9 ("ImportFromIMDb::showData (3x const Glib::ustring&, IMDbProgress*) - " << name);
   Check1 (progress); Check1 (client);
   progress->hide ();
   Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::removeProgressBar), client, progress));

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
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showError (const Glib::ustring& msg, IMDbProgress* progress) {
   Gtk::MessageDialog (msg, Gtk::MESSAGE_ERROR).run ();
   Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::removeProgressBar), client, progress));

   status = QUERY;
   inputChanged ();
   txtID->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Shows the results of an IMDb-search
/// \param results Map containing found entries (ID/name)
/// \param progress Progress bar used for displaying the status; will be hidden
//-----------------------------------------------------------------------------
void ImportFromIMDb::showSearchResults (const std::map<Glib::ustring, Glib::ustring>& results,
					IMDbProgress* progress) {
   Check1 (progress); Check1 (client);
   progress->hide ();
   Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::stopLoading), progress));

   MovieColumns colMovies;
   Glib::RefPtr<Gtk::ListStore> model (Gtk::ListStore::create (colMovies));
   Gtk::TreeView& list (*new Gtk::TreeView (model));

   // Fill the lines into the list
   for (std::map<Glib::ustring, Glib::ustring>::const_iterator i (results.begin ());
	i != results.end (); ++i) {
      Gtk::TreeModel::Row row (*model->append ());
      row[colMovies.id] = i->first;
      row[colMovies.name] = i->second;
   }

   list.append_column (_("Movie"), colMovies.name);
   list.show ();
   client->attach (*manage (&list), 0, 2, 2, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   list.grab_focus ();
   list.get_selection ()->signal_changed ().connect
      (bind (mem_fun (*this, &ImportFromIMDb::rowSelected), &list));

   status = CHOOSING;
   ok->signal_clicked ().connect (bind (mem_fun (*this, &ImportFromIMDb::continueLoading),
					&list, progress));
}

//-----------------------------------------------------------------------------
/// Continues loading with the selected list-entry
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::continueLoading (Gtk::TreeView* list, IMDbProgress* progress) {
   Check1 (list); Check1 (progress); Check2 (client);

   Gtk::TreeModel::Row row (*list->get_selection ()->get_selected ());
   MovieColumns colMovies;
   progress->start (row[colMovies.id]);
   list->hide ();
   client->remove (*list);
}
