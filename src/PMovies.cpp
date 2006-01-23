//$Id: PMovies.cpp,v 1.1 2006/01/23 04:04:52 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Movies
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
//COPYRIGHT   : Copyright (C) 2006

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <cdmgr-cfg.h>

#if WITH_ACTORS == 1

#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include "LangImg.h"
#include "StorageMovie.h"

#include "PMovies.h"


//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling movies
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
//-----------------------------------------------------------------------------
PMovies::PMovies (Gtk::Statusbar& status, Gtk::Widget& menuSave, const Genres& genres)
   : NBPage (status, menuSave), imgLang (NULL), movies (genres), relMovies ("movies")
{
   Gtk::ScrolledWindow* scrlMovies (new Gtk::ScrolledWindow);
   scrlMovies->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlMovies->add (movies);
   scrlMovies->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   movies.signalOwnerChanged.connect (mem_fun (*this, &PMovies::directorChanged));
   movies.signalObjectChanged.connect (mem_fun (*this, &PMovies::entityChanged));
   movies.signalNameChanged.connect (mem_fun (*this, &PMovies::movieNameChanged));

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
   HDirector director;
   director.define ();
   directors.push_back (director);

   Gtk::TreeModel::iterator i (movies.append (director));
   movies.selectRow (i);
   directorChanged (director);
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
   TRACE9 ("void PMovies::newMovie () - Founding director " << movies.getDirectorAt (p)->getName ());

   HMovie movie;
   movie.define ();
   Gtk::TreeIter i (movies.append (movie, *p));
   movies.expand_row (movies.getModel ()->get_path (p), false);
   movies.selectRow (i);

   HDirector director;
   director = movies.getDirectorAt (p);
   relMovies.relate (director, movie);

   movieChanged (movie);
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
/// \param director: Handle to changed director
//-----------------------------------------------------------------------------
void PMovies::directorChanged (const HDirector& director) {
   TRACE9 ("PDirectors::directorChanged (const HDirector&)");

   if (changedDirectors.find (director) == changedDirectors.end ()) {
      changedDirectors[director] = new Director (*director);
      enableSave ();
   }
   else
      undoDirectors.erase (std::find (undoDirectors.begin (), undoDirectors.end (), director));
   apMenus[UNDO]->set_sensitive ();

   undoDirectors.push_back (director);
}

//----------------------------------------------------------------------------
/// Callback when changing a movie
/// \param movie: Handle to changed movie
//-----------------------------------------------------------------------------
void PMovies::entityChanged (const HEntity& movie) {
   TRACE9 ("PMovies::movieChanged (const HEntity&)");
   movieChanged (HMovie::cast (movie));
}

//----------------------------------------------------------------------------
/// Callback when changing a movie
/// \param movie: Handle to changed movie
//-----------------------------------------------------------------------------
void PMovies::movieChanged (const HMovie& movie) {
   TRACE9 ("PMovies::movieChanged (const HMovie&)");

   if (changedMovies.find (movie) == changedMovies.end ()) {
      changedMovies[movie] = new Movie (*movie);
      enableSave ();
   }
   else
      undoMovies.erase (std::find (undoMovies.begin (), undoMovies.end (), movie));
   apMenus[UNDO]->set_sensitive ();

   undoMovies.push_back (movie);
}

//-----------------------------------------------------------------------------
/// Callback (additional to movieChanged) when the name of a movie is being
/// changed
/// \param movie: Handle to changed movie
//-----------------------------------------------------------------------------
void PMovies::movieNameChanged (const HMovie& movie) {
   Check3 (movie.isDefined ());
   TRACE9 ("PMovies::movieNameChanged (const HMovie&) - " << movie->getId () << '/' << movie->getName ());

   changedMovieNames[movie] += std::string (1, ',');
   changedMovieNames[movie] += Movie::currLang;
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
   Check3 (!imgLang);
   imgLang = new LanguageImg (Movie::currLang.c_str ());
   imgLang->show ();
   imgLang->signal_clicked ().connect (mem_fun (*this, &PMovies::selectLanguage));

   statusbar.pack_end (*imgLang, Gtk::PACK_SHRINK, 5);

   ui += ("<menuitem action='NDirector'/><menuitem action='NMovie'/>"
	  "</placeholder></menu><placeholder name='Lang'><menu action='Lang'>");

   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NDirector", Gtk::Stock::NEW,
							_("New _director")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PMovies::newDirector));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NMovie", _("_New movie")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PMovies::newMovie));
   apMenus[NEW3].clear ();

   grpAction->add (Gtk::Action::create ("Lang", _("_Language")));
   addLanguageMenus (ui, grpAction);
   ui += "</menu></placeholder>";
   movieSelected ();
}

//-----------------------------------------------------------------------------
/// Removes any created page-related menus
//-----------------------------------------------------------------------------
void PMovies::removeMenu () {
   statusbar.remove (*imgLang);
   delete imgLang;
   imgLang = NULL;
}

//-----------------------------------------------------------------------------
/// Callback after selecting menu to set the language in which the
/// movies are displayed
/// \param lang: Lanuage in which the movies should be displayed
//-----------------------------------------------------------------------------
void PMovies::changeLanguage (const std::string& lang) {
   Check3 (lang.empty () || (lang.size () == 2));

   static bool ignore (false);                  // Ignore de-selecting an entry
   ignore = !ignore;
   TRACE1 ("PMovies::changeLanguage (const std::string&) - " << lang << ": "
	   << (ignore ? "True" : "False"));
   if (ignore)
      return;

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
/// Sets the focus to the actor-list
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
      Check3 (d->isDefined ());
      const std::vector<HMovie>& movies (relMovies.getObjects (*d));

      for (std::vector<HMovie>::const_iterator m (movies.begin ()); m != movies.end (); ++m) {
	 Check3 (m->isDefined ());
	 if ((*m)->getId () == id)
	    return *m;
      }
   }
   HMovie m;
   return m;
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
      StorageMovie::loadMovies (aMovies, stat);
      TRACE8 ("PMovies::loadData () - Found " << aMovies.size () << " movies");

      for (std::vector<HDirector>::const_iterator i (directors.begin ());
	   i != directors.end (); ++i) {
	 Check3 (i->isDefined ());
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

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 movie", "Loaded %1 movies", aMovies.size ())));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (aMovies.size ()));
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
//-----------------------------------------------------------------------------
void PMovies::saveData () throw (Glib::ustring) {
   HDirector director;

   try {
      while (changedDirectors.size ()) {
	 director = changedDirectors.begin ()->first;
	 Check3 (director.isDefined ());
	 Check3 (changedDirectors.begin ()->second);

	 StorageMovie::saveDirector (director);
	 changedDirectors.erase (changedDirectors.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't write director `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, director->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   HMovie movie;
   try {
      while (changedMovies.size ()) {
	 movie = changedMovies.begin ()->first; Check3 (movie.isDefined ());
	 Check3 (relMovies.isRelated (movie));

	 StorageMovie::saveMovie (movie, relMovies.getParent (movie)->getId ());

	 StorageMovie::startTransaction ();
	 StorageMovie::deleteMovieNames (movie->getId ());

	 YGP::Tokenize langs (changedMovieNames[movie]);
	 std::string lang;
	 while ((lang = langs.getNextNode (',')).size ()) {
	    Check3 (Language::exists (lang));

	    if (movie->getName (lang).size ())
	       StorageMovie::saveMovieName (movie, lang);
	 } // endwhile languages changed
	 StorageMovie::commitTransaction ();
      }
      Check3 (changedMovies.begin ()->second);
      changedMovies.erase (changedMovies.begin ());
   }
   catch (std::exception& err) {
      StorageMovie::abortTransaction ();

      Glib::ustring msg (_("Can't write movie `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, movie->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedMovies.size ()) {
	 movie = deletedMovies.begin ()->first; Check3 (movie.isDefined ());
	 if (movie->getId ())
	    StorageMovie::deleteMovie (movie->getId ());
	 deletedMovies.erase (deletedMovies.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete movie `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, movie->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedDirectors.size ()) {
	 director = *deletedDirectors.begin (); Check3 (director.isDefined ());
	 if (director->getId ())
	    StorageMovie::deleteDirector (director->getId ());
	 deletedDirectors.erase (deletedDirectors.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete director `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, director->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }
}

#endif
