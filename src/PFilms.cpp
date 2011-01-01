//PROJECT     : CDManager
//SUBSYSTEM   : Films
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009, 2010

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

#include <sstream>

#include <unistd.h>

#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include "LangImg.h"
#include "SaveCeleb.h"
#include "ImportIMDb.h"
#include "StorageMovie.h"

#include "PMovies.h"


//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling films
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
//-----------------------------------------------------------------------------
PMovies::PMovies (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres)
   : NBPage (status, menuSave), imgLang (NULL), movies (genres), relMovies ("movies")
{
   TRACE9 ("PMovies::PMovies (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&)");

   Gtk::ScrolledWindow* scrlMovies (new Gtk::ScrolledWindow);
   scrlMovies->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlMovies->add (movies);
   scrlMovies->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   movies.signalOwnerChanged.connect (mem_fun (*this, &PMovies::directorChanged));
   movies.signalObjectChanged.connect (mem_fun (*this, &PMovies::movieChanged));

   Check3 (movies.get_selection ());
   movies.get_selection ()->signal_changed ().connect (mem_fun (*this, &PMovies::movieSelected));

   widget = scrlMovies;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PMovies::~PMovies () {
}


//-----------------------------------------------------------------------------
/// Adds a new direcotor to the list
//-----------------------------------------------------------------------------
void PMovies::newDirector () {
   TRACE5 ("void PMovies::newDirector ()");
   HDirector director (new Director);
   Gtk::TreeModel::iterator i (addDirector (director));

   Gtk::TreePath path (movies.getModel ()->get_path (i));
   movies.set_cursor (path, *movies.get_column (0), true);
}

//-----------------------------------------------------------------------------
/// Adds the passed director to the list
/// \param hDirector Director to add
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PMovies::addDirector (HDirector& hDirector) {
   Check1 (hDirector);
   TRACE5 ("void PMovies::addDirector () - " << hDirector->getName ());

   directors.push_back (hDirector);
   Gtk::TreeModel::iterator i (movies.append (hDirector));
   Gtk::TreePath path (movies.getModel ()->get_path (i));
   movies.selectRow (i);

   aUndo.push (Undo (Undo::INSERT, DIRECTOR, 0, hDirector, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a new movie to the first selected director
//-----------------------------------------------------------------------------
void PMovies::newMovie () {
   TRACE5 ("void PMovies::newMovie ()");

   Glib::RefPtr<Gtk::TreeSelection> movieSel (movies.get_selection ()); Check3 (movieSel);
   Gtk::TreeIter p (movieSel->get_selected ()); Check3 (p);
   if ((*p)->parent ())
      p = ((*p)->parent ());
   TRACE9 ("void PMovies::newMovie () - Found director " << movies.getDirectorAt (p)->getName ());

   HMovie movie (new Movie);
   Gtk::TreeIter i (addMovie (movie, *p));
   Gtk::TreePath path (movies.getModel ()->get_path (i));
   movies.set_cursor (path, *movies.get_column (0), true);
}

//-----------------------------------------------------------------------------
/// Adds a new movie to the first selected director
/// \param hMovie Movie to add
/// \param pos Position in list where to add the movie
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PMovies::addMovie (HMovie& hMovie, Gtk::TreeIter pos) {
   Check1 (hMovie); Check1 (pos);
   TRACE9 ("PMovies::addMovie (HMovie&, Gtk::TreeIter) - " << hMovie->getName ());

   Gtk::TreeIter i (movies.append (hMovie, *pos));
   Gtk::TreePath path (movies.getModel ()->get_path (i));
   movies.expand_row (movies.getModel ()->get_path (pos), false);
   movies.selectRow (i);

   HDirector director;
   director = movies.getDirectorAt (pos);
   relMovies.relate (director, hMovie);

   aUndo.push (Undo (Undo::INSERT, MOVIE, 0, hMovie, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Callback after selecting a movie
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PMovies::movieSelected () {
   TRACE9 ("PMovies::movieSelected ()");
   Check3 (movies.get_selection ());

   Gtk::TreeIter s (movies.get_selection ()->get_selected ());
   enableEdit (s ? OWNER_SELECTED : NONE_SELECTED);
}

//----------------------------------------------------------------------------
/// Callback when changing a director
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PMovies::directorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PDirectors::directorChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)\n\t- " << column << '/' << oldValue);

   Gtk::TreePath path (movies.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, DIRECTOR, column, movies.getCelebrityAt (row), path, oldValue));

   enableSave ();
   apMenus[UNDO]->set_sensitive ();
}

//----------------------------------------------------------------------------
/// Callback when changing a movie
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PMovies::movieChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PMovies::movieChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)\n\t- " << column << '/' << oldValue);

   Gtk::TreePath path (movies.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, MOVIE, column, movies.getObjectAt (row), path, oldValue));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Adds the menu-entries for the language-menu to the passed string
/// \param menu: String, where to add the language-entries to
/// \param grpAction: Actiongroup to use
//-----------------------------------------------------------------------------
void PMovies::addLanguageMenus (Glib::ustring& menu, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   TRACE9 ("PMovies::addLanguageMenus (Glib::ustring&)");

   menu += "<menuitem action='Orig'/>";

   Gtk::RadioButtonGroup grpLang;
   grpAction->add (Gtk::RadioAction::create (grpLang, "Orig", _("_Original name")),
		   Gtk::AccelKey ("<ctl>0"),
		   bind (mem_fun (*this, &PMovies::changeLanguage), ""));

   char accel[7];
   strcpy (accel, "<ctl>1");
   for (std::map<std::string, Language>::const_iterator i (Language::begin ());
	i != Language::end (); ++i) {
      TRACE9 ("PMovies::PMovies (Options&) - Adding language " << i->first);
      menu += "<menuitem action='" + i->first + "'/>";

      Glib::RefPtr<Gtk::RadioAction> act
	 (Gtk::RadioAction::create (grpLang, i->first, i->second.getInternational ()));

      if (accel[5] <= '9') {
	 grpAction->add (act, Gtk::AccelKey (accel),
			 bind (mem_fun (*this, &PMovies::changeLanguage), i->first));
	 ++accel[5];
      }
      else
	 grpAction->add (act, bind (mem_fun (*this, &PMovies::changeLanguage), i->first));

      if (i->first == Movie::currLang)
	 act->set_active ();
   }
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PMovies::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   TRACE9 ("PMovies::addMenu (Glib::ustring&, Glib::RefPtr<Gtk::ActionGroup>");
   Check3 (!imgLang);
   imgLang = new LanguageImg (Movie::currLang.c_str ());
   imgLang->show ();
   imgLang->signal_clicked ().connect (mem_fun (*this, &PMovies::selectLanguage));

   statusbar.pack_end (*imgLang, Gtk::PACK_SHRINK, 5);

   ui += ("<menuitem action='MUndo'/>"
	  "<separator/>"
	  "<menuitem action='NDirector'/>"
	  "<menuitem action='NMovie'/>"
	  "<separator/>"
	  "<menuitem action='MDelete'/>"
	  "<separator/>"
	  "<menuitem action='MImport'/>"
	  "<menuitem action='MImportDescr'/>"
	  "</placeholder></menu>"
	  "<placeholder name='Other'><menu action='Lang'>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("MUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &PMovies::undo));
   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NDirector", Gtk::Stock::NEW,
							_("New _director")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PMovies::newDirector));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NMovie", _("_New movie")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PMovies::newMovie));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("MDelete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PMovies::deleteSelection));
   grpAction->add (Gtk::Action::create ("MImport", _("_Import from IMDb.com ...")),
		   Gtk::AccelKey (_("<ctl>I")), mem_fun (*this, &PMovies::importFromIMDb));
   grpAction->add (Gtk::Action::create ("MImportDescr", _("_Import descriptions from IMDb.com ...")),
		   Gtk::AccelKey (_("<shft><ctl>I")), mem_fun (*this, &PMovies::importDescriptionFromIMDb));

   grpAction->add (Gtk::Action::create ("Lang", _("_Language")));
   addLanguageMenus (ui, grpAction);
   ui += "</menu></placeholder>";

   apMenus[UNDO]->set_sensitive (false);
   movieSelected ();
}

//-----------------------------------------------------------------------------
/// Removes page-related menus
//-----------------------------------------------------------------------------
void PMovies::removeMenu () {
   TRACE9 ("PMovies::removeMenu ()");
   if (imgLang) {
      statusbar.remove (*imgLang);
      delete imgLang;
      imgLang = NULL;
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting menu to set the language in which the
/// movies are displayed
/// \param lang: Lanuage in which the movies should be displayed
//-----------------------------------------------------------------------------
void PMovies::changeLanguage (const std::string& lang) {
   Check3 (lang.empty () || (lang.size () == 2));

   TRACE1 ("PMovies::changeLanguage (const std::string&) - " << lang);
   if (lang != Movie::currLang)
      setLanguage (lang);
}

//-----------------------------------------------------------------------------
/// Changes the language in which the movies are displayed
/// \param lang: Lanuage in which the movies should be displayed
//-----------------------------------------------------------------------------
void PMovies::setLanguage (const std::string& lang) {
   TRACE9 ("PMovies::setLanguage (const std::string&) - " << lang);
   Movie::currLang = lang;
   if ((lang.size () == 2) && !loadedLangs[lang])
      loadData (lang);

   movies.update (lang);
   imgLang->update (lang.c_str ());
}

//-----------------------------------------------------------------------------
/// Callback to select the language in which to display the movies
//-----------------------------------------------------------------------------
void PMovies::selectLanguage () {
   TRACE9 ("PMovies::selectLanguage ()");

   Glib::RefPtr<Gtk::UIManager> mgrUI (Gtk::UIManager::create ());
   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   Glib::ustring ui ("<ui><popup name='PopupLang'>");
   addLanguageMenus (ui, grpAction);
   ui += "</popup></ui>";

   mgrUI->insert_action_group (grpAction);
   mgrUI->add_ui_from_string (ui);

   Gtk::Menu* popup (dynamic_cast<Gtk::Menu*> (mgrUI->get_widget ("/PopupLang")));
   popup->popup (0, gtk_get_current_event_time ());
}

//-----------------------------------------------------------------------------
/// Sets the focus to the movie-list
//-----------------------------------------------------------------------------
void PMovies::getFocus () {
   movies.grab_focus ();
}

//-----------------------------------------------------------------------------
/// Finds the movie with the passed id
/// \param directors: Vector of known directors
/// \param relMovies: Relation of above directors to their movies
/// \param id: Id of movie to find
/// \returns HMovie: Found movie (undefined, if not found)
//-----------------------------------------------------------------------------
HMovie PMovies::findMovie (const std::vector<HDirector>& directors,
			   const YGP::Relation1_N<HDirector, HMovie>& relMovies,
			   unsigned int id) {
   for (std::vector<HDirector>::const_iterator d (directors.begin ());
	d != directors.end (); ++d) {
      Check3 (*d);
      const std::vector<HMovie>& movies (relMovies.getObjects (*d));

      for (std::vector<HMovie>::const_iterator m (movies.begin ()); m != movies.end (); ++m) {
	 Check3 (*m);
	 if ((*m)->getId () == id)
	    return *m;
      }
   }
   return HMovie ();
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database.
//-----------------------------------------------------------------------------
void PMovies::loadData () {
   TRACE9 ("PMovies::loadData ()");
   try {
      YGP::StatusObject stat;
      StorageMovie::loadDirectors (directors, stat);
      std::sort (directors.begin (), directors.end (), &Director::compByName);

      std::map<unsigned int, std::vector<HMovie> > aMovies;
      unsigned int cMovies (StorageMovie::loadMovies (aMovies, stat));
      TRACE8 ("PMovies::loadData () - Found " << cMovies << " movies");

      for (std::vector<HDirector>::const_iterator i (directors.begin ());
	   i != directors.end (); ++i) {
	 Check3 (*i);
	 Gtk::TreeModel::Row director (movies.append (*i));

	 std::map<unsigned int, std::vector<HMovie> >::iterator iMovie
	    (aMovies.find ((*i)->getId ()));
	 if (iMovie != aMovies.end ()) {
	    for (std::vector<HMovie>::iterator m (iMovie->second.begin ());
		 m != iMovie->second.end (); ++m) {
	       movies.append (*m, director);
	       relMovies.relate (*i, *m);
	    } // end-for all movies for a director
	    aMovies.erase (iMovie);
	 } // end-if director has movies
      } // end-for all directors

      movies.expand_all ();

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 movie", "Loaded %1 movies", cMovies)));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cMovies));
      showStatus (msg);

      loaded = true;

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading movies"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database for a certain language.
///
/// According to the available information the page of the notebook
/// is created.
/// \param lang: Language for movies to load
//-----------------------------------------------------------------------------
void PMovies::loadData (const std::string& lang) {
   TRACE5 ("PMovies::loadMovies (const std::string&) - Language: " << lang);
   try {
      StorageMovie::loadNames (directors, relMovies, lang);
      loadedLangs[lang] = true;
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PMovies::saveData () throw (std::exception) {
   TRACE9 ("PMovies::saveData ()");

   std::vector<HEntity> aSaved;
   std::vector<HEntity>::iterator posSaved (aSaved.end ());

   // Save all data in the undo-buffer. Successfully saved data is removed
   // this buffer. This means sucessful saving disables undo
   while (aUndo.size ()) {
      Undo last (aUndo.top ());

      // Only save each entries once (when if it is changed more then once)
      posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
      if ((posSaved == aSaved.end ()) || (*posSaved != last.getEntity ())) {
	 switch (last.what ()) {
	 case MOVIE: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Movie));
	    HMovie movie (boost::dynamic_pointer_cast<Movie> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (movie->getId ()) {
		  Check3 (movie->getId () == last.column ());
		  StorageMovie::deleteMovie (movie->getId ());
	       }

	       std::map<HEntity, HEntity>::iterator delRel
		  (delRelation.find (last.getEntity ()));
	       Check3 (delRel != delRelation.end ());
	       Check3 (typeid (*delRel->second) == typeid (Director));
	       delRelation.erase (delRel);
	    }
	    else {
	       HDirector director  (relMovies.getParent (movie));
	       if (!director->getId ()) {
		  Check3 (std::find (aSaved.begin (), aSaved.end (), director) == aSaved.end ());
		  Check3 (delRelation.find (director) == delRelation.end ());

		  SaveCelebrity::store (director, "Directors", *getWindow ());
		  aSaved.insert (lower_bound (aSaved.begin (), aSaved.end (), director), director);
		  posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
	       }
	       StorageMovie::saveMovie (movie, relMovies.getParent (movie)->getId ());
	    }
	    break; }

	 case DIRECTOR: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Director));
	    HDirector director (boost::dynamic_pointer_cast<Director> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (director->getId ()) {
		  Check3 (director->getId () == last.column ());
		  StorageMovie::deleteDirector (director->getId ());
	       }
	    }
	    else
	       SaveCelebrity::store (director, "Directors", *getWindow ());
	    break; }

	 default:
	    Check1 (0);
	 } // end-switch
	 aSaved.insert (posSaved, last.getEntity ());
	 }
      aUndo.pop ();
   } // end-while
   Check3 (apMenus[UNDO]);
   apMenus[UNDO]->set_sensitive (false);

   Check3 (delRelation.empty ());
}

//-----------------------------------------------------------------------------
/// Removes the selected movies or directors from the listbox. Depending movies
/// are deleted too.
//-----------------------------------------------------------------------------
void PMovies::deleteSelection () {
   TRACE9 ("PMovies::deleteSelection ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (movies.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (movies.get_model ()->get_iter (*i)); Check3 (iter);
      if ((*iter)->parent ())                 // A movie is going to be deleted
	 deleteMovie (iter);
      else {                               // A director is going to be deleted
	 TRACE9 ("PMovies::deleteSelection () - Deleting " << iter->children ().size () << " children");
	 HDirector director (movies.getDirectorAt (iter)); Check3 (director);
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    deleteMovie (child);
	 }

	 Gtk::TreePath path (movies.getModel ()->get_path (iter));
	 aUndo.push (Undo (Undo::DELETE, DIRECTOR, director->getId (), director, path, ""));
	 movies.getModel ()->erase (iter);
      }
   }
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Deletes the passed movie
/// \param movie: Iterator to movie to delete
//-----------------------------------------------------------------------------
void PMovies::deleteMovie (const Gtk::TreeIter& movie) {
   Check2 (movie->children ().empty ());

   HMovie hMovie (movies.getMovieAt (movie));
   TRACE9 ("PMovies::deleteMovie (const Gtk::TreeIter&) - Deleting movie "
	   << hMovie->getName ());
   Check3 (relMovies.isRelated (hMovie));
   HDirector hDirector (relMovies.getParent (hMovie)); Check3 (hDirector);

   Check3 (delRelation.find (hMovie) == delRelation.end ());

   Glib::RefPtr<Gtk::TreeStore> model (movies.getModel ());
   Gtk::TreePath path (model->get_path (movies.getOwner (hDirector)));
   aUndo.push (Undo (Undo::DELETE, MOVIE, hMovie->getId (), hMovie, path, ""));
   delRelation[hMovie] = hDirector;

   relMovies.unrelate (hDirector, hMovie);
   model->erase (movie);
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
/// \param lang: Language, in which to export
//-----------------------------------------------------------------------------
void PMovies::export2HTML (unsigned int fd, const std::string& lang) {
   std::string oldLang (Movie::currLang);
   Movie::currLang = lang;

   // Load the names of the movies in the actual language
   if (!loadedLangs[Movie::currLang])
      loadData (Movie::currLang);
   Check3 (loadedLangs[Movie::currLang]);

   std::sort (directors.begin (), directors.end (), &Director::compByName);

   // Write movie-information
   for (std::vector<HDirector>::const_iterator i (directors.begin ());
	i != directors.end (); ++i)
      if (relMovies.isRelated (*i)) {
	 std::stringstream output;
	 output << 'D' << **i;

	 const std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	 Check3 (dirMovies.size ());
	 for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
	      m != dirMovies.end (); ++m)
	    output << "M" << **m;

	 TRACE9 ("PMovies::export2HTML (unsinged int) - Writing: " << output.str ());
	 if (::write (fd, output.str ().data (), output.str ().size ()) != (ssize_t)output.str ().size ()) {
	    Glib::ustring msg (_("Couldn't write data!\n\nReason: %1"));
	    msg.replace (msg.find ("%1"), 2, strerror (errno));
	    Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	    dlg.set_title (_("Error exporting movies to HTML!"));
	    dlg.run ();
	    break;
	 }
      }

   Movie::currLang = oldLang;
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PMovies::undo () {
   TRACE1 ("PMovies::undo ()");
   Check3 (aUndo.size ());

   Undo last (aUndo.top ());
   switch (last.what ()) {
   case MOVIE:
      undoMovie (last);
      break;

   case DIRECTOR:
      undoDirector (last);
      break;

   default:
      Check1 (0);
   } // end-switch

   aUndo.pop ();
   if (aUndo.empty ()) {
      enableSave (false);
      apMenus[UNDO]->set_sensitive (false);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a movie
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PMovies::undoMovie (const Undo& last) {
   TRACE5 ("PMovies::undoMovie (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (movies.getModel ()->get_iter (path));

   Check3 (typeid (*last.getEntity ()) == typeid (Movie));
   HMovie movie (boost::dynamic_pointer_cast<Movie> (last.getEntity ()));
   TRACE9 ("PMovies::undoMovie (const Undo&) - " << last.how () << ": " << movie->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      Check3 (iter->parent ());

      switch (last.column ()) {
      case 0:
	 movie->setName (last.getValue ());
	 break;

      case 1:
	 movie->setYear (last.getValue ());
	 break;

      case 2:
	 movie->setGenre ((unsigned int)last.getValue ()[0]);
	 break;

      case 3:
	 movie->setType (last.getValue ());
	 break;

      case 4:
	 movie->setLanguage (last.getValue ());
	 break;

      case 5:
	 movie->setTitles (last.getValue ());
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (iter->parent ());
      Check3 (relMovies.isRelated (movie));
      relMovies.unrelate (movies.getDirectorAt (iter->parent ()), movie);
      movies.getModel ()->erase (iter);
      iter = movies.getModel ()->children ().end ();
      break;

   case Undo::DELETE: {
      std::map<HEntity, HEntity>::iterator delRel (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Director));
      HDirector director (boost::dynamic_pointer_cast<Director> (delRel->second));
      Gtk::TreeRow rowDirector (*movies.getOwner (director));

      iter = movies.append (movie, rowDirector);
      path = movies.getModel ()->get_path (iter);

      relMovies.relate (director, movie);

      delRelation.erase (delRel);
      break; }

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      movies.update (row);
   }
   movies.set_cursor (path);
   movies.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a director
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PMovies::undoDirector (const Undo& last) {
   TRACE5 ("PMovies::undoDirector (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (movies.getModel ()->get_iter (path));

   Check1 (last.getEntity ());
   Check3 (typeid (*last.getEntity ()) == typeid (Director));
   HDirector director (boost::dynamic_pointer_cast<Director> (last.getEntity ()));
   TRACE9 ("PMovies::undoDirector (const Undo&) - " << last.how () << ": " << director->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      Check3 (iter); Check3 (!iter->parent ());

      switch (last.column ()) {
      case 0:
	 director->setName (last.getValue ());
	 break;

      case 1:
	 director->setLifespan (last.getValue ());
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (iter); Check3 (!iter->parent ());
      Check3 (!relMovies.isRelated (director));
      movies.getModel ()->erase (iter);
      iter = movies.getModel ()->children ().end ();
      break;

   case Undo::DELETE:
      if (iter)
	 Check3 (!iter->parent ());
      else
	 iter = movies.getModel ()->children ().end ();
      iter = movies.insert (director, iter);
      path = movies.getModel ()->get_path (iter);
      break;

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      movies.update (row);
   }
   movies.set_cursor (path);
   movies.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PMovies::clear () {
   relMovies.unrelateAll ();
   directors.clear ();
   movies.clear ();
   movies.getModel ()->clear ();
   NBPage::clear ();
   loadedLangs.clear ();
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for a movie from IMDb.com
//-----------------------------------------------------------------------------
void PMovies::importFromIMDb () {
   TRACE9 ("PMovies::importFromIMDb ()");
   ImportFromIMDb* dlg (ImportFromIMDb::create ());
   dlg->sigLoaded.connect (mem_fun (*this, &PMovies::importMovie));
   dlg->signal_response ().connect (bind (ptr_fun (&PMovies::closeDialog), dlg));
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for a movie from IMDb.com
//-----------------------------------------------------------------------------
void PMovies::importDescriptionFromIMDb () {
   TRACE9 ("PMovies::importDescriptionFromIMDb ()");
   std::vector<HMovie> iMovies;
   for (std::vector<HDirector>::const_iterator d (directors.begin ());
	d != directors.end(); ++d) {
      const std::vector<HMovie>& dMovies (relMovies.getObjects(*d));
      for (std::vector<HMovie>::const_iterator m (dMovies.begin ());
	   m != dMovies.end (); ++m)
	 if ((*m)->getDescription ().empty ())
	    iMovies.push_back (*m);
   }
   TRACE5 ("PMovies::importDescriptionFromIMDb () - To process: " << iMovies.size ());

   ImportFromIMDb* dlg (ImportFromIMDb::create ());
   dlg->sigLoaded.connect (mem_fun (*this, &PMovies::importMovie));
   dlg->signal_response ().connect (bind (ptr_fun (&PMovies::closeDialog), dlg));
}

//-----------------------------------------------------------------------------
/// Frees the passed dialog
/// \param int Response of dialog (ignored)
/// \param dlg Dialog to close additionally
//-----------------------------------------------------------------------------
void PMovies::closeDialog (int, const Gtk::Dialog* dlg) {
   Check1 (dlg);
   delete dlg;
}

//-----------------------------------------------------------------------------
/// Imports the passed movie-information and adds them appropiately to the
/// list of movies.
/// Algorithm: If the name of the director does exist, ask if they are equal.
///    The movie is added to the respective director
/// \param director Name of the director
/// \param movie Name of the movie with year in parenthesis at the end
/// \param genre Genre of the movie
/// \param summary Synopsis of the film
//-----------------------------------------------------------------------------
bool PMovies::importMovie (const Glib::ustring& director, const Glib::ustring& movie,
			   const Glib::ustring& genre, const Glib::ustring& summary) {
   TRACE5 ("PMovies::importMovie (4x const Glib::ustring&) - " << director << ": " << movie);

   Glib::ustring nameMovie (movie);
   YGP::AYear year;

   // Strip trailing type of movie (IMDb adds " (x)" for certain types of movies)
   if ((nameMovie.size () > 4) && (nameMovie[nameMovie.size () - 1] == ')')
       && (nameMovie[nameMovie.size () - 3] == '(') && (nameMovie[nameMovie.size () - 4] == ' '))
      nameMovie = nameMovie.substr (0, nameMovie.size () - 4);

   // Check if the movie has the year in parenthesis appended
   std::string::size_type pos (nameMovie.rfind (" (", nameMovie.size () - 2));
   if ((nameMovie[nameMovie.size () - 1] == ')') && (pos != std::string::npos)) {
      try {
	 year = nameMovie.substr (pos + 2, nameMovie.size () - pos - 3);
	 nameMovie = nameMovie.substr (0, pos);
      }
      catch (std::invalid_argument& err) { }
   }

   // Create the new director
   HDirector hDirector (new Director);
   hDirector->setName (director);

   Gtk::TreeModel::const_iterator iNewDirector (movies.getOwner (director));
   if (iNewDirector) {
      std::vector<HDirector> sameDirectors;
      std::map<unsigned long, Gtk::TreeModel::const_iterator> iDirectors;

      // Get all directors with a similar name
      for (Gtk::TreeModel::const_iterator i (movies.getModel ()->children ().begin ());
	   i != movies.getModel ()->children ().end (); ++i) {
	 HDirector actDirector (movies.getDirectorAt (i));
	 if (!actDirector->getName ().compare (0, director.size (), director)) {
	    iDirectors[actDirector->getId ()] = i;
	    sameDirectors.push_back (actDirector);
	 }
      }

      boost::scoped_ptr<SaveCelebrity> dlgCeleb
	 (SaveCelebrity::create (*(Gtk::Window*)getWindow ()->get_toplevel (), hDirector, sameDirectors));
      switch (dlgCeleb->run ()) {
      case Gtk::RESPONSE_YES:
	 TRACE9 ("PMovies::importMovie (3x const Glib::ustring&) - Same director " << dlgCeleb->getIdOfSelection ());
	 Check3 (dlgCeleb->getIdOfSelection ());
	 hDirector->setId (dlgCeleb->getIdOfSelection ());
	 iNewDirector = iDirectors[dlgCeleb->getIdOfSelection ()];
	 break;

      case Gtk::RESPONSE_NO:
	 iNewDirector = addDirector (hDirector);
	 break;

      default:
	 return false;
      }
   }
   else
      iNewDirector = addDirector (hDirector);

   int idGenre (movies.getGenre (genre));
   if (idGenre == -1)
      idGenre = 0;

   HMovie hMovie (new Movie);
   hMovie->setGenre (idGenre);
   hMovie->setName (nameMovie);
   hMovie->setYear (year);
   hMovie->setDescription (summary);
   addMovie (hMovie, iNewDirector);
   return true;
}
