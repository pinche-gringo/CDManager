//PROJECT     : CDManager
//SUBSYSTEM   : Films
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009 - 2011

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
#include "StorageFilm.h"

#include "PFilms.h"


//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling films
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
//-----------------------------------------------------------------------------
PFilms::PFilms (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres)
   : NBPage (status, menuSave), imgLang (NULL), films (genres), relFilms ("films")
{
   TRACE9 ("PFilms::PFilms (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&)");

   Gtk::ScrolledWindow* scrlFilms (new Gtk::ScrolledWindow);
   scrlFilms->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlFilms->add (films);
   scrlFilms->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   films.signalOwnerChanged.connect (mem_fun (*this, &PFilms::directorChanged));
   films.signalObjectChanged.connect (mem_fun (*this, &PFilms::filmChanged));

   Check3 (films.get_selection ());
   films.get_selection ()->signal_changed ().connect (mem_fun (*this, &PFilms::filmSelected));

   widget = scrlFilms;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PFilms::~PFilms () {
}


//-----------------------------------------------------------------------------
/// Adds a new direcotor to the list
//-----------------------------------------------------------------------------
void PFilms::newDirector () {
   TRACE5 ("void PFilms::newDirector ()");
   HDirector director (new Director);
   Gtk::TreeModel::iterator i (addDirector (director));

   Gtk::TreePath path (films.getModel ()->get_path (i));
   films.set_cursor (path, *films.get_column (0), true);
}

//-----------------------------------------------------------------------------
/// Adds the passed director to the list
/// \param hDirector Director to add
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PFilms::addDirector (HDirector& hDirector) {
   Check1 (hDirector);
   TRACE5 ("void PFilms::addDirector () - " << hDirector->getName ());

   directors.push_back (hDirector);
   Gtk::TreeModel::iterator i (films.append (hDirector));
   Gtk::TreePath path (films.getModel ()->get_path (i));
   films.selectRow (i);

   aUndo.push (Undo (Undo::INSERT, DIRECTOR, 0, hDirector, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a new film to the first selected director
//-----------------------------------------------------------------------------
void PFilms::newFilm () {
   TRACE5 ("void PFilms::newFilm ()");

   Glib::RefPtr<Gtk::TreeSelection> filmSel (films.get_selection ()); Check3 (filmSel);
   Gtk::TreeIter p (filmSel->get_selected ()); Check3 (p);
   if ((*p)->parent ())
      p = ((*p)->parent ());
   TRACE9 ("void PFilms::newFilm () - Found director " << films.getDirectorAt (p)->getName ());

   HFilm film (new Film);
   Gtk::TreeIter i (addFilm (film, *p));
   Gtk::TreePath path (films.getModel ()->get_path (i));
   films.set_cursor (path, *films.get_column (0), true);
}

//-----------------------------------------------------------------------------
/// Adds a new film to the first selected director
/// \param hFilm Film to add
/// \param pos Position in list where to add the film
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PFilms::addFilm (HFilm& hFilm, Gtk::TreeIter pos) {
   Check1 (hFilm); Check1 (pos);
   TRACE9 ("PFilms::addFilm (HFilm&, Gtk::TreeIter) - " << hFilm->getName ());

   Gtk::TreeIter i (films.append (hFilm, *pos));
   Gtk::TreePath path (films.getModel ()->get_path (i));
   films.expand_row (films.getModel ()->get_path (pos), false);
   films.selectRow (i);

   HDirector director;
   director = films.getDirectorAt (pos);
   relFilms.relate (director, hFilm);

   aUndo.push (Undo (Undo::INSERT, FILM, 0, hFilm, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Callback after selecting a film
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PFilms::filmSelected () {
   TRACE9 ("PFilms::filmSelected ()");
   Check3 (films.get_selection ());

   Gtk::TreeIter s (films.get_selection ()->get_selected ());
   enableEdit (s ? OWNER_SELECTED : NONE_SELECTED);
}

//----------------------------------------------------------------------------
/// Callback when changing a director
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PFilms::directorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PFilms::directorChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)\n\t- " << column << '/' << oldValue);

   Gtk::TreePath path (films.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, DIRECTOR, column, films.getCelebrityAt (row), path, oldValue));

   enableSave ();
   apMenus[UNDO]->set_sensitive ();
}

//----------------------------------------------------------------------------
/// Callback when changing a film
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PFilms::filmChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PFilms::filmChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)\n\t- " << column << '/' << oldValue);

   Gtk::TreePath path (films.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, FILM, column, films.getObjectAt (row), path, oldValue));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Adds the menu-entries for the language-menu to the passed string
/// \param menu: String, where to add the language-entries to
/// \param grpAction: Actiongroup to use
//-----------------------------------------------------------------------------
void PFilms::addLanguageMenus (Glib::ustring& menu, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   TRACE9 ("PFilms::addLanguageMenus (Glib::ustring&)");

   menu += "<menuitem action='Orig'/>";

   Gtk::RadioButtonGroup grpLang;
   grpAction->add (Gtk::RadioAction::create (grpLang, "Orig", _("_Original name")),
		   Gtk::AccelKey ("<ctl>0"),
		   bind (mem_fun (*this, &PFilms::changeLanguage), ""));

   char accel[7];
   strcpy (accel, "<ctl>1");
   for (std::map<std::string, Language>::const_iterator i (Language::begin ());
	i != Language::end (); ++i) {
      TRACE9 ("PFilms::PFilms (Options&) - Adding language " << i->first);
      menu += "<menuitem action='" + i->first + "'/>";

      Glib::RefPtr<Gtk::RadioAction> act
	 (Gtk::RadioAction::create (grpLang, i->first, i->second.getInternational ()));

      if (accel[5] <= '9') {
	 grpAction->add (act, Gtk::AccelKey (accel),
			 bind (mem_fun (*this, &PFilms::changeLanguage), i->first));
	 ++accel[5];
      }
      else
	 grpAction->add (act, bind (mem_fun (*this, &PFilms::changeLanguage), i->first));

      if (i->first == Film::currLang)
	 act->set_active ();
   }
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PFilms::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   TRACE9 ("PFilms::addMenu (Glib::ustring&, Glib::RefPtr<Gtk::ActionGroup>");
   Check3 (!imgLang);
   imgLang = new LanguageImg (Film::currLang.c_str ());
   imgLang->show ();
   imgLang->signal_clicked ().connect (mem_fun (*this, &PFilms::selectLanguage));

   statusbar.pack_end (*imgLang, Gtk::PACK_SHRINK, 5);

   ui += ("<menuitem action='MUndo'/>"
	  "<separator/>"
	  "<menuitem action='NDirector'/>"
	  "<menuitem action='NFilm'/>"
	  "<separator/>"
	  "<menuitem action='MDelete'/>"
	  "<separator/>"
	  "<menuitem action='MImport'/>"
	  "<menuitem action='MImportDescr'/>"
	  "</placeholder></menu>"
	  "<placeholder name='Other'><menu action='Lang'>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("MUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &PFilms::undo));
   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NDirector", Gtk::Stock::NEW,
							_("New _director")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PFilms::newDirector));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NFilm", _("_New film")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PFilms::newFilm));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("MDelete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PFilms::deleteSelection));
   grpAction->add (Gtk::Action::create ("MImport", _("_Import from IMDb.com ...")),
		   Gtk::AccelKey (_("<ctl>I")), mem_fun (*this, &PFilms::importFromIMDb));
   grpAction->add (Gtk::Action::create ("MImportDescr", _("_Import information from IMDb.com ...")),
		   Gtk::AccelKey (_("<shft><ctl>I")), mem_fun (*this, &PFilms::importInfoFromIMDb));

   grpAction->add (Gtk::Action::create ("Lang", _("_Language")));
   addLanguageMenus (ui, grpAction);
   ui += "</menu></placeholder>";

   apMenus[UNDO]->set_sensitive (false);
   filmSelected ();
}

//-----------------------------------------------------------------------------
/// Removes page-related menus
//-----------------------------------------------------------------------------
void PFilms::removeMenu () {
   TRACE9 ("PFilms::removeMenu ()");
   if (imgLang) {
      statusbar.remove (*imgLang);
      delete imgLang;
      imgLang = NULL;
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting menu to set the language in which the
/// films are displayed
/// \param lang: Lanuage in which the films should be displayed
//-----------------------------------------------------------------------------
void PFilms::changeLanguage (const std::string& lang) {
   Check3 (lang.empty () || (lang.size () == 2));

   TRACE1 ("PFilms::changeLanguage (const std::string&) - " << lang);
   if (lang != Film::currLang)
      setLanguage (lang);
}

//-----------------------------------------------------------------------------
/// Changes the language in which the films are displayed
/// \param lang: Lanuage in which the films should be displayed
//-----------------------------------------------------------------------------
void PFilms::setLanguage (const std::string& lang) {
   TRACE9 ("PFilms::setLanguage (const std::string&) - " << lang);
   Film::currLang = lang;
   if ((lang.size () == 2) && !loadedLangs[lang])
      loadData (lang);

   films.update (lang);
   imgLang->update (lang.c_str ());
}

//-----------------------------------------------------------------------------
/// Callback to select the language in which to display the films
//-----------------------------------------------------------------------------
void PFilms::selectLanguage () {
   TRACE9 ("PFilms::selectLanguage ()");

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
/// Sets the focus to the film-list
//-----------------------------------------------------------------------------
void PFilms::getFocus () {
   films.grab_focus ();
}

//-----------------------------------------------------------------------------
/// Finds the film with the passed id
/// \param directors: Vector of known directors
/// \param relFilms: Relation of above directors to their films
/// \param id: Id of film to find
/// \returns HFilm: Found film (undefined, if not found)
//-----------------------------------------------------------------------------
HFilm PFilms::findFilm (const std::vector<HDirector>& directors,
			   const YGP::Relation1_N<HDirector, HFilm>& relFilms,
			   unsigned int id) {
   for (std::vector<HDirector>::const_iterator d (directors.begin ());
	d != directors.end (); ++d) {
      Check3 (*d);
      const std::vector<HFilm>& films (relFilms.getObjects (*d));

      for (std::vector<HFilm>::const_iterator m (films.begin ()); m != films.end (); ++m) {
	 Check3 (*m);
	 if ((*m)->getId () == id)
	    return *m;
      }
   }
   return HFilm ();
}

//-----------------------------------------------------------------------------
/// Loads the films from the database.
//-----------------------------------------------------------------------------
void PFilms::loadData () {
   TRACE9 ("PFilms::loadData ()");
   try {
      YGP::StatusObject stat;
      StorageFilm::loadDirectors (directors, stat);
      std::sort (directors.begin (), directors.end (), &Director::compByName);

      std::map<unsigned int, std::vector<HFilm> > aFilms;
      unsigned int cFilms (StorageFilm::loadFilms (aFilms, stat));
      TRACE8 ("PFilms::loadData () - Found " << cFilms << " films");

      for (std::vector<HDirector>::const_iterator i (directors.begin ());
	   i != directors.end (); ++i) {
	 Check3 (*i);
	 Gtk::TreeModel::Row director (films.append (*i));

	 std::map<unsigned int, std::vector<HFilm> >::iterator iFilm
	    (aFilms.find ((*i)->getId ()));
	 if (iFilm != aFilms.end ()) {
	    for (std::vector<HFilm>::iterator m (iFilm->second.begin ());
		 m != iFilm->second.end (); ++m) {
	       films.append (*m, director);
	       relFilms.relate (*i, *m);
	    } // end-for all films for a director
	    aFilms.erase (iFilm);
	 } // end-if director has films
      } // end-for all directors

      films.expand_all ();

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 film", "Loaded %1 films", cFilms)));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cFilms));
      showStatus (msg);

      loaded = true;

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading films"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available films!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog (msg, false, Gtk::MESSAGE_ERROR).run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the films from the database for a certain language.
///
/// According to the available information the page of the notebook
/// is created.
/// \param lang: Language for films to load
//-----------------------------------------------------------------------------
void PFilms::loadData (const std::string& lang) {
   TRACE5 ("PFilms::loadFilms (const std::string&) - Language: " << lang);
   try {
      StorageFilm::loadNames (directors, relFilms, lang);
      loadedLangs[lang] = true;
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available films!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog (msg, false, Gtk::MESSAGE_ERROR).run ();
   }
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PFilms::saveData () throw (std::exception) {
   TRACE9 ("PFilms::saveData ()");

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
	 case FILM: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Film));
	    HFilm film (boost::dynamic_pointer_cast<Film> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (film->getId ()) {
		  Check3 (film->getId () == last.column ());
		  StorageFilm::deleteFilm (film->getId ());
	       }

	       std::map<HEntity, HEntity>::iterator delRel
		  (delRelation.find (last.getEntity ()));
	       Check3 (delRel != delRelation.end ());
	       Check3 (typeid (*delRel->second) == typeid (Director));
	       delRelation.erase (delRel);
	    }
	    else {
	       HDirector director  (relFilms.getParent (film));
	       if (!director->getId ()) {
		  Check3 (std::find (aSaved.begin (), aSaved.end (), director) == aSaved.end ());
		  Check3 (delRelation.find (director) == delRelation.end ());

		  SaveCelebrity::store (director, "Directors", *getWindow ());
		  aSaved.insert (lower_bound (aSaved.begin (), aSaved.end (), director), director);
		  posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
	       }
	       StorageFilm::saveFilm (film, relFilms.getParent (film)->getId ());
	    }
	    break; }

	 case DIRECTOR: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Director));
	    HDirector director (boost::dynamic_pointer_cast<Director> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (director->getId ()) {
		  Check3 (director->getId () == last.column ());
		  StorageFilm::deleteDirector (director->getId ());
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
/// Removes the selected films or directors from the listbox. Depending films
/// are deleted too.
//-----------------------------------------------------------------------------
void PFilms::deleteSelection () {
   TRACE9 ("PFilms::deleteSelection ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (films.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (films.get_model ()->get_iter (*i)); Check3 (iter);
      if ((*iter)->parent ())                 // A film is going to be deleted
	 deleteFilm (iter);
      else {                               // A director is going to be deleted
	 TRACE9 ("PFilms::deleteSelection () - Deleting " << iter->children ().size () << " children");
	 HDirector director (films.getDirectorAt (iter)); Check3 (director);
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    deleteFilm (child);
	 }

	 Gtk::TreePath path (films.getModel ()->get_path (iter));
	 aUndo.push (Undo (Undo::DELETE, DIRECTOR, director->getId (), director, path, ""));
	 films.getModel ()->erase (iter);
      }
   }
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Deletes the passed film
/// \param film: Iterator to film to delete
//-----------------------------------------------------------------------------
void PFilms::deleteFilm (const Gtk::TreeIter& film) {
   Check2 (film->children ().empty ());

   HFilm hFilm (films.getFilmAt (film));
   TRACE9 ("PFilms::deleteFilm (const Gtk::TreeIter&) - Deleting film "
	   << hFilm->getName ());
   Check3 (relFilms.isRelated (hFilm));
   HDirector hDirector (relFilms.getParent (hFilm)); Check3 (hDirector);

   Check3 (delRelation.find (hFilm) == delRelation.end ());

   Glib::RefPtr<Gtk::TreeStore> model (films.getModel ());
   Gtk::TreePath path (model->get_path (films.getOwner (hDirector)));
   aUndo.push (Undo (Undo::DELETE, FILM, hFilm->getId (), hFilm, path, ""));
   delRelation[hFilm] = hDirector;

   relFilms.unrelate (hDirector, hFilm);
   model->erase (film);
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
/// \param lang: Language, in which to export
//-----------------------------------------------------------------------------
void PFilms::export2HTML (unsigned int fd, const std::string& lang) {
   std::string oldLang (Film::currLang);
   Film::currLang = lang;

   // Load the names of the films in the actual language
   if (!loadedLangs[Film::currLang])
      loadData (Film::currLang);
   Check3 (loadedLangs[Film::currLang]);

   std::sort (directors.begin (), directors.end (), &Director::compByName);

   // Write film-information
   for (std::vector<HDirector>::const_iterator i (directors.begin ());
	i != directors.end (); ++i)
      if (relFilms.isRelated (*i)) {
	 std::stringstream output;
	 output << 'D' << **i;

	 const std::vector<HFilm>& dirFilms (relFilms.getObjects (*i));
	 Check3 (dirFilms.size ());
	 for (std::vector<HFilm>::const_iterator m (dirFilms.begin ());
	      m != dirFilms.end (); ++m)
	    output << "M" << **m;

	 TRACE9 ("PFilms::export2HTML (unsinged int) - Writing: " << output.str ());
	 if (::write (fd, output.str ().data (), output.str ().size ()) != (ssize_t)output.str ().size ()) {
	    Glib::ustring msg (_("Couldn't write data!\n\nReason: %1"));
	    msg.replace (msg.find ("%1"), 2, strerror (errno));
	    Gtk::MessageDialog dlg (msg, false, Gtk::MESSAGE_ERROR);
	    dlg.set_title (_("Error exporting films to HTML!"));
	    dlg.run ();
	    break;
	 }
      }

   Film::currLang = oldLang;
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PFilms::undo () {
   TRACE1 ("PFilms::undo ()");
   Check3 (aUndo.size ());

   Undo last (aUndo.top ());
   switch (last.what ()) {
   case FILM:
      undoFilm (last);
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
/// Undoes the last changes to a film
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PFilms::undoFilm (const Undo& last) {
   TRACE5 ("PFilms::undoFilm (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (films.getModel ()->get_iter (path));

   Check3 (typeid (*last.getEntity ()) == typeid (Film));
   HFilm film (boost::dynamic_pointer_cast<Film> (last.getEntity ()));
   TRACE9 ("PFilms::undoFilm (const Undo&) - " << last.how () << ": " << film->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      Check3 (iter->parent ());

      switch (last.column ()) {
      case 0:
	 film->setName (last.getValue ());
	 break;

      case 1:
	 film->setYear (last.getValue ());
	 break;

      case 2:
	 film->setGenre ((unsigned int)last.getValue ()[0]);
	 break;

      case 3:
	 film->setType (last.getValue ());
	 break;

      case 4:
	 film->setLanguage (last.getValue ());
	 break;

      case 5:
	 film->setTitles (last.getValue ());
	 break;

      case 99:
	 film->setDescription ("");
	 film->setImage ("");
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (iter->parent ());
      Check3 (relFilms.isRelated (film));
      relFilms.unrelate (films.getDirectorAt (iter->parent ()), film);
      films.getModel ()->erase (iter);
      iter = films.getModel ()->children ().end ();
      break;

   case Undo::DELETE: {
      std::map<HEntity, HEntity>::iterator delRel (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Director));
      HDirector director (boost::dynamic_pointer_cast<Director> (delRel->second));
      Gtk::TreeRow rowDirector (*films.getOwner (director));

      iter = films.append (film, rowDirector);
      path = films.getModel ()->get_path (iter);

      relFilms.relate (director, film);

      delRelation.erase (delRel);
      break; }

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      films.update (row);
   }
   films.set_cursor (path);
   films.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a director
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PFilms::undoDirector (const Undo& last) {
   TRACE5 ("PFilms::undoDirector (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (films.getModel ()->get_iter (path));

   Check1 (last.getEntity ());
   Check3 (typeid (*last.getEntity ()) == typeid (Director));
   HDirector director (boost::dynamic_pointer_cast<Director> (last.getEntity ()));
   TRACE9 ("PFilms::undoDirector (const Undo&) - " << last.how () << ": " << director->getName ());

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
      Check3 (!relFilms.isRelated (director));
      films.getModel ()->erase (iter);
      iter = films.getModel ()->children ().end ();
      break;

   case Undo::DELETE:
      if (iter)
	 Check3 (!iter->parent ());
      else
	 iter = films.getModel ()->children ().end ();
      iter = films.insert (director, iter);
      path = films.getModel ()->get_path (iter);
      break;

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      films.update (row);
   }
   films.set_cursor (path);
   films.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PFilms::clear () {
   relFilms.unrelateAll ();
   directors.clear ();
   films.clear ();
   films.getModel ()->clear ();
   NBPage::clear ();
   loadedLangs.clear ();
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for a film from IMDb.com
//-----------------------------------------------------------------------------
void PFilms::importFromIMDb () {
   TRACE9 ("PFilms::importFromIMDb ()");
   ImportFromIMDb* dlg (ImportFromIMDb::create ());
   dlg->sigLoaded.connect (mem_fun (*this, &PFilms::importFilm));
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for films from IMDb.com
//-----------------------------------------------------------------------------
void PFilms::importInfoFromIMDb () {
   TRACE9 ("PFilms::importDescriptionFromIMDb ()");
   std::vector<HFilm>* iFilms (new std::vector<HFilm>);
   for (std::vector<HDirector>::const_iterator d (directors.begin ());
	d != directors.end(); ++d) {
      const std::vector<HFilm>& dFilms (relFilms.getObjects(*d));
      for (std::vector<HFilm>::const_iterator m (dFilms.begin ());
	   m != dFilms.end (); ++m)
	 if ((*m)->getDescription ().empty () || (*m)->getImage ().empty ())
	    iFilms->push_back (*m);
   }
   TRACE5 ("PFilms::importDescriptionFromIMDb () - To process: " << iFilms->size ());

   ImportFromIMDb* dlg (ImportFromIMDb::create ());
   dlg->sigLoaded.connect (bind (mem_fun (*this, &PFilms::continousImportFilm), dlg, iFilms));
   importNextFilm (dlg, iFilms);
}

//-----------------------------------------------------------------------------
/// Imports the last entry of the passed list of films
/// \param dlg Dialog displaying the import-information
/// \param films Pointer to list of all films to import
//-----------------------------------------------------------------------------
bool PFilms::importNextFilm (ImportFromIMDb* dlg, std::vector<HFilm>* films) {
   if (films->size ()) {
      TRACE5 ("PFilms::importNextFilm (ImportFromIMDb*, std::vector<HFilm>*) - " << films->back ()->getName ());
      dlg->searchFor (films->back ()->getName ());
   }
   else
      delete films;
   return false;
}

//-----------------------------------------------------------------------------
/// Imports the passed film-information and adds them appropiately to the
/// list of films.
/// Algorithm: If the name of the director does exist, ask if they are equal.
///    The film is added to the respective director
/// \param director Name of the director
/// \param film Name of the film with year in parenthesis at the end
/// \param genre Genre of the film
/// \param summary Synopsis of the film
/// \param image Poster of the film
/// \param dlg Dialog to load elements
/// \param filmlist List of films to load
//-----------------------------------------------------------------------------
bool PFilms::continousImportFilm (const Glib::ustring& director, const Glib::ustring& film,
				  const Glib::ustring& genre, const Glib::ustring& summary,
				  const std::string& image, ImportFromIMDb* dlg, std::vector<HFilm>* filmlist) {
   TRACE3 ("PFilms::continousImportFilm (4x const Glib::ustring&, const std::string&, std::vector<HFilm>*) - " << filmlist->back ()->getName () << '/' << image.size ());
   Check1 (film->size ());

   std::vector<HFilm>::iterator last (filmlist->end () - 1);
   TRACE5 ("PFilms::continousImportFilm (4x const Glib::ustring&, const std::string&, std::vector<HFilm>*) - "
	   << (*last)->getName () << "->" << relFilms.getParent (*last)->getName ());
   if (director.empty () || (director == relFilms.getParent (*last)->getName ())) {
      bool changed (((*last)->getDescription ().empty () && summary.size () && ((*last)->setDescription (summary), true)));
      if (((*last)->getImage ().empty () && image.size () && ((*last)->setImage (image), true)) || changed) {
	 const Gtk::TreeIter row (films.getObject (*last));
	 Glib::ustring empty;
	 filmChanged (row, 99, empty);
      }
   }
   else {
      Glib::ustring msg (_("Not setting data from IMDb for '%1' as the director differs (found %2, should be %3)!"));
      msg.replace (msg.find ("%1"), 2, (*last)->getName ());
      msg.replace (msg.find ("%2"), 2, director);
      msg.replace (msg.find ("%3"), 2, relFilms.getParent (*last)->getName ());
      if (Gtk::MessageDialog (msg, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK_CANCEL).run ()
	  == Gtk::RESPONSE_CANCEL)
	 return false;
   }

   filmlist->erase (last);
   Glib::signal_idle ().connect (bind (mem_fun (*this, &PFilms::importNextFilm), dlg, filmlist));
   return false;
}

//-----------------------------------------------------------------------------
/// Imports the passed film-information and adds them appropiately to the
/// list of films.
/// Algorithm: If the name of the director does exist, ask if they are equal.
///    The film is added to the respective director
/// \param director Name of the director
/// \param film Name of the film with year in parenthesis at the end
/// \param genre Genre of the film
/// \param summary Synopsis of the film
/// \param image Poster of the film
//-----------------------------------------------------------------------------
bool PFilms::importFilm (const Glib::ustring& director, const Glib::ustring& film,
			 const Glib::ustring& genre, const Glib::ustring& summary,
			 const std::string& image) {
   TRACE5 ("PFilms::importFilm (4x const Glib::ustring&, const std::string&) - " << director << ": " << film);

   Glib::ustring nameFilm (film);
   YGP::AYear year;

   // Strip trailing type of film (IMDb adds " (x)" for certain types of films)
   if ((nameFilm.size () > 4) && (nameFilm[nameFilm.size () - 1] == ')')
       && (nameFilm[nameFilm.size () - 3] == '(') && (nameFilm[nameFilm.size () - 4] == ' '))
      nameFilm = nameFilm.substr (0, nameFilm.size () - 4);

   // Check if the film has the year in parenthesis appended
   std::string::size_type pos (nameFilm.rfind (" (", nameFilm.size () - 2));
   if ((nameFilm[nameFilm.size () - 1] == ')') && (pos != std::string::npos)) {
      try {
	 year = nameFilm.substr (pos + 2, nameFilm.size () - pos - 3);
	 nameFilm = nameFilm.substr (0, pos);
      }
      catch (std::invalid_argument& err) { }
   }

   // Create the new director
   HDirector hDirector (new Director);
   hDirector->setName (director);

   Gtk::TreeModel::const_iterator iNewDirector (films.getOwner (director));
   if (iNewDirector) {
      std::vector<HDirector> sameDirectors;
      std::map<unsigned long, Gtk::TreeModel::const_iterator> iDirectors;

      // Get all directors with a similar name
      for (Gtk::TreeModel::const_iterator i (films.getModel ()->children ().begin ());
	   i != films.getModel ()->children ().end (); ++i) {
	 HDirector actDirector (films.getDirectorAt (i));
	 if (!actDirector->getName ().compare (0, director.size (), director)) {
	    iDirectors[actDirector->getId ()] = i;
	    sameDirectors.push_back (actDirector);
	 }
      }

      boost::scoped_ptr<SaveCelebrity> dlgCeleb
	 (SaveCelebrity::create (*(Gtk::Window*)getWindow ()->get_toplevel (), hDirector, sameDirectors));
      switch (dlgCeleb->run ()) {
      case Gtk::RESPONSE_YES:
	 TRACE9 ("PFilms::importFilm (3x const Glib::ustring&) - Same director " << dlgCeleb->getIdOfSelection ());
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

   int idGenre (films.getGenre (genre));
   if (idGenre == -1)
      idGenre = 0;

   HFilm hFilm (new Film);
   hFilm->setGenre (idGenre);
   hFilm->setName (nameFilm);
   hFilm->setYear (year);
   hFilm->setDescription (summary);
   hFilm->setImage (image);
   addFilm (hFilm, iNewDirector);
   return true;
}
