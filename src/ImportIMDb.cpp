//PROJECT     : CDManager
//SUBSYSTEM   : Films
//REFERENCES  :
//TODO        : - Used edit-fields instead of labels; to reuse dialog for edit of info
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 19.03.2010
//COPYRIGHT   : Copyright (C) 2010, 2011

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

#include <gdkmm/pixbufloader.h>

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "IMDbProgress.h"

#include "ImportIMDb.h"


/**Class defining the columns for the list of results of the IMDb-search
 */
class FilmColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   FilmColumns () : Gtk::TreeModel::ColumnRecord () { add (id); add (name); }

   Gtk::TreeModelColumn<std::string> id;
   Gtk::TreeModelColumn<Glib::ustring> name;
};


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb ()
   : XGP::XDialog (XGP::XDialog::NONE), sigLoaded (), client (new Gtk::Table (7, 2)),
     txtID (new Gtk::Entry), lblDirector (new Gtk::Label (Glib::ustring (), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP)),
     lblFilm (new Gtk::Label (Glib::ustring (), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP)),
     lblGenre (new Gtk::Label (Glib::ustring (), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP)),
     lblSummary (new Gtk::Label (Glib::ustring (), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP)),
     image (new Gtk::Image ()), status (QUERY), connOK () {
   set_title (_("Import from IMDb.com"));

   client->show ();

   Gtk::Label* lbl (new Gtk::Label (_("Film (_Name, number or URL):"), true));
   lbl->set_mnemonic_widget (*txtID);
   client->attach (*manage (lbl), 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
   client->attach (*txtID, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK, 5, 5);

   get_vbox ()->pack_start (*client, true, true, 5);

   txtID->set_activates_default ();
   txtID->signal_changed ().connect (mem_fun (*this, &ImportFromIMDb::inputChanged));

   ok = new Gtk::Button (Gtk::Stock::GO_FORWARD);
   get_action_area ()->pack_start (*ok, false, false, 5);
   ok->set_flags (Gtk::CAN_DEFAULT);
   ok->grab_default ();
   ok->signal_clicked ().connect (mem_fun (*this, &ImportFromIMDb::okEvent));

   cancel = add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

   inputChanged ();

   show_all_children ();

   lbl = new Gtk::Label (_("Director:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP);
   client->attach (*manage (lbl), 0, 1, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   client->attach (*manage (lblDirector), 1, 2, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   lbl = new Gtk::Label (_("Film:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP);
   client->attach (*manage (lbl), 0, 1, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   client->attach (*manage (lblFilm), 1, 2, 3, 4, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   lbl = new Gtk::Label (_("Genre:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP);
   client->attach (*manage (lbl), 0, 1, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   client->attach (*manage (lblGenre), 1, 2, 4, 5, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

   lbl = new Gtk::Label (_("Plot summary:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP);
   client->attach (*manage (lbl), 0, 1, 5, 6, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);
   lblSummary->set_line_wrap ();
   client->attach (*manage (lblSummary), 0, 3, 6, 7, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5, 5);

   client->attach (*manage (image), 2, 3, 0, 6, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK, 5, 5);

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
   Gtk::TreeIter sel (list->get_selection ()->get_selected ());
   if (sel) {
      Gtk::TreeModel::Row row (*sel);
      TRACE7 ("ImportFromIMDb::rowSelected (Gtk::TreeView*) - " << row.parent ())
      ok->set_sensitive (row.parent ());
   }
   else
      ok->set_sensitive (false);
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

      IMDbProgress* progress (new IMDbProgress (Glib::locale_from_utf8 (txtID->get_text ())));
      progress->sigError.connect (bind (mem_fun (*this, &ImportFromIMDb::showError), progress));
      progress->sigAmbiguous.connect (bind (mem_fun (*this, &ImportFromIMDb::showSearchResults), progress));
      progress->sigSuccess.connect (bind (mem_fun (*this, &ImportFromIMDb::showData), progress));
      progress->show ();
      client->attach (*manage (progress), 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND,
		     Gtk::FILL | Gtk::SHRINK, 5, 5);
      break;
   }

   case CHOOSING:
      // Do nothing; any action is performed by other handlers
      break;

   case CONFIRM:
      if (saveIMDbInfo ())
	 response (Gtk::RESPONSE_OK);
      break;

   default:
      TRACE1 ("Status: " << status);
      Check (0);
   }
}

//-----------------------------------------------------------------------------
/// Informs listeners about the information in the dialog
/// \returns bool True, if all listeners successfully handled the update
//-----------------------------------------------------------------------------
bool ImportFromIMDb::saveIMDbInfo () {
   Check3 (lblDirector); Check3 (lblFilm); Check3 (lblGenre); Check3 (image);
   gchar buffer[8192];
   gchar* gbuf (buffer);
   gsize bufSize (sizeof(buffer));
   image->get_pixbuf ()->save_to_buffer(gbuf, bufSize, "jpeg");

   std::string strBuffer (buffer, bufSize);
   return sigLoaded.emit (lblDirector->get_text (), lblFilm->get_text (), lblGenre->get_text (),
			  lblSummary->get_text (), strBuffer);
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
   stopLoading (progress);
   client->remove (*progress);
   delete progress;
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
/// Adds an icon to the film information
/// \param bufImage Image description
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::addIcon (const std::string& bufImage, IMDbProgress* progress) {
   TRACE1 ("ImportFromIMDb::addIcon (const std::string&, IMDbProgress*) - " << bufImage.length ());

   Glib::RefPtr<Gdk::PixbufLoader>  picLoader (Gdk::PixbufLoader::create ());
   try {
      picLoader->write ((const guint8*)bufImage.data (), (gsize)bufImage.size ());
      picLoader->close ();
      image->set (picLoader->get_pixbuf ()->scale_simple (87, 128, Gdk::INTERP_BILINEAR));
      TRACE9 ("ImportFromIMDb::addIcon (const std::string&, IMDbProgress*) - Dimensions: " << image->get_width () << '/' << image->get_height ());
      image->show ();
   }
   catch (Gdk::PixbufError& e) { }
   catch (Glib::FileError& e) { }
   catch (Glib::Error& e) {
      TRACE1 ("Error: " << e.what ());
   }

   status = CONFIRM;
   progress->hide ();
   Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::removeProgressBar), client, progress));
}

//-----------------------------------------------------------------------------
/// Adds an icon to the film information
/// \param image Image description
/// \param progress Progress bar used for displaying the status; will be removed
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool ImportFromIMDb::loadIcon (const std::string& image, IMDbProgress* progress) {
   TRACE1 ("ImportFromIMDb::loadIcon (const std::string&, IMDbProgress*) - " << image);
   Check1 (progress); Check1 (image.size ());
   status = IMGLOAD;
   progress->start (image, true);
   progress->sigIcon.connect (bind (mem_fun (*this, &ImportFromIMDb::addIcon), progress));
   return false;
}

//-----------------------------------------------------------------------------
/// Updates the dialog with the data read
/// \param entry Found IMDbEntry
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showData (const IMDbProgress::IMDbEntry& entry, IMDbProgress* progress) {
   TRACE9 ("ImportFromIMDb::showData (3x const Glib::ustring&, IMDbProgress*) - " << entry.title);
   Check1 (progress); Check1 (client);

   if (entry.image.size ()) {
      Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::stopLoading), progress));
      Glib::signal_idle ().connect (bind (mem_fun (*this, &ImportFromIMDb::loadIcon), entry.image, progress));
   }
   else {
      status = CONFIRM;
      progress->hide ();
      Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::removeProgressBar), client, progress));
   }

   lblDirector->set_text (entry.director);
   lblFilm->set_text (entry.title);
   lblGenre->set_text (entry.genre);
   lblSummary->set_text (entry.summary);
   show_all_children ();

   ok->set_label (Gtk::Stock::OK.id);
   ok->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
/// \param msg Message to display
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showError (const Glib::ustring& msg, IMDbProgress* progress) {
   if (status != IMGLOAD) {
      TRACE9 ("ImportFromIMDb::showError (const Glib::ustring&, IMDbProgress*) - " << msg);
      Gtk::MessageDialog (msg, Gtk::MESSAGE_ERROR).run ();
      Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::removeProgressBar), client, progress));

      status = QUERY;
      inputChanged ();
      txtID->set_sensitive ();
   }
}

//-----------------------------------------------------------------------------
/// Shows the results of an IMDb-search
/// \param results Map containing found entries (ID/name)
/// \param progress Progress bar used for displaying the status; will be hidden
//-----------------------------------------------------------------------------
void ImportFromIMDb::showSearchResults (const std::map<IMDbProgress::match, IMDbProgress::IMDbSearchEntries>& results,
					IMDbProgress* progress) {
   Check1 (progress); Check1 (client);
   progress->hide ();
   Glib::signal_idle ().connect (bind (ptr_fun (&ImportFromIMDb::stopLoading), progress));

   FilmColumns colFilms;
   Glib::RefPtr<Gtk::TreeStore> model (Gtk::TreeStore::create (colFilms));
   Gtk::ScrolledWindow& scrl (*new Gtk::ScrolledWindow);
   Gtk::TreeView& list (*new Gtk::TreeView (model));

   // Fill the lines into the list
   Glib::ustring matches[] = { _("Popular Titles"), _("Exact match"), _("Partial match"), _("Approximate match") };
   for (unsigned int i (0); i < (sizeof (matches) / sizeof (matches[0])); ++i) {
      const IMDbProgress::IMDbSearchEntries& films (results.at ((IMDbProgress::match)i));
      if (films.begin () != films.end ()) {
	 Gtk::TreeModel::Row match (*model->append ());
	 match[colFilms.name] = matches[i];

	 for (IMDbProgress::IMDbSearchEntries::const_iterator m (films.begin ()); m != films.end (); ++m) {
	    Gtk::TreeModel::Row row (*model->append (match.children ()));
	    row[colFilms.id] = m->url;
	    row[colFilms.name] = m->title;
	 }

	 if (i != ((sizeof (matches) / sizeof (matches[0]))) - 1)
	    list.expand_row (model->get_path (match), false);
      }
   }

   scrl.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrl.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrl.add (*manage (&list));
   list.append_column (_("Film"), colFilms.name);
   list.set_size_request (-1, 150);
   scrl.show ();
   list.show ();
   list.signal_row_activated ().connect (bind (mem_fun (*this, &ImportFromIMDb::rowActivated),
					       &scrl, &list, progress));

   client->attach (*manage (&scrl), 0, 2, 2, 5, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5, 5);

   list.grab_focus ();
   list.get_selection ()->signal_changed ().connect
      (bind (mem_fun (*this, &ImportFromIMDb::rowSelected), &list));

   status = CHOOSING;
   connOK = ok->signal_clicked ().connect (bind (mem_fun (*this, &ImportFromIMDb::continueLoading),
						 &scrl, &list, progress));
}

//-----------------------------------------------------------------------------
/// Continues loading with the selected list-entry
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::continueLoading (Gtk::ScrolledWindow* scrl, Gtk::TreeView* list,
				      IMDbProgress* progress) {
   Gtk::TreeRow row (*list->get_selection ()->get_selected ());
   if (!row)
      return;

   loadRow (row, scrl, list, progress);
}

//-----------------------------------------------------------------------------
/// Continues with loading the film identified by the passed row
/// \param row Row to load
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::loadRow (Gtk::TreeRow& row, Gtk::ScrolledWindow* scrl,
			      Gtk::TreeView* list, IMDbProgress* progress) {
   Check1 (scrl); Check1 (list); Check1 (progress); Check2 (client);

   Check3 (connOK.connected ());
   connOK.disconnect ();
   scrl->hide ();
   client->remove (*scrl);

   progress->show ();
   progress->start (row[FilmColumns ().id]);
}

//-----------------------------------------------------------------------------
/// Callback after double-clicking a row; continues loading this entry
/// \param path Activated path
/// \param column Column in path
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::rowActivated (const Gtk::TreePath& path, Gtk::TreeViewColumn* column,
				   Gtk::ScrolledWindow* scrl, Gtk::TreeView* list,
				   IMDbProgress* progress) {
   Check1 (list);
   Gtk::TreeRow row (*(list->get_model ()->get_iter (path)));
   loadRow (row, scrl, list, progress);
}

//-----------------------------------------------------------------------------
/// Searches for the passed film
/// \param film Information of the film to search for
//-----------------------------------------------------------------------------
void ImportFromIMDb::searchFor (const Glib::ustring& film) {
   txtID->set_text (film);
   status = QUERY;
   okEvent ();
}
