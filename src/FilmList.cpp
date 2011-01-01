//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011

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

#include <cerrno>
#include <cstdlib>

#include <boost/tokenizer.hpp>

#include <gtkmm/cellrenderercombo.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "Genres.h"
#include "CDType.h"
#include "LangDlg.h"

#include "FilmList.h"


typedef boost::tokenizer<boost::char_separator<char> > tokenizer;


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
FilmList::FilmList (const Genres& genres)
   : OwnerObjectList (genres)
     , mTypes (Gtk::ListStore::create (colTypes)) {
   TRACE9 ("FilmList::FilmList (const Genres&)");
   mOwnerObjects = Gtk::TreeStore::create (colFilms);
   init (colFilms);

   // Add column "Type"
   Gtk::CellRendererCombo* renderer (new Gtk::CellRendererCombo ());
   renderer->property_text_column () = 0;
   renderer->property_model () = mTypes;
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn
				(_("Type"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text (), colFilms.type);
   column->add_attribute (renderer->property_visible(), colFilms.chgAll);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &FilmList::valueChanged), 0));

   CDType& type (CDType::getInstance ());
   for (CDType::const_iterator t (type.begin ()); t != type.end (); ++t) {
      Gtk::TreeModel::Row newType (*mTypes->append ());
      newType[colTypes.type] = (t->second);
   }

   // Add column "Languages"
   column = new Gtk::TreeViewColumn (_("Language(s)"));
   column->pack_start (colFilms.lang1, false);
   column->pack_start (colFilms.lang2, false);
   column->pack_start (colFilms.lang3, false);
   column->pack_start (colFilms.lang4, false);
   column->pack_start (colFilms.lang5, false);

   append_column (*Gtk::manage (column));
   column->set_resizable ();
   column->add_attribute (column->get_first_cell_renderer ()
			  ->property_visible(), colFilms.chgAll);

   // Add column "Subtitles"
   column = new Gtk::TreeViewColumn (_("Subtitles(s)"));
   column->pack_start (colFilms.sub1, false);
   column->pack_start (colFilms.sub2, false);
   column->pack_start (colFilms.sub3, false);
   column->pack_start (colFilms.sub4, false);
   column->pack_start (colFilms.sub5, false);
   column->pack_start (colFilms.sub6, false);
   column->pack_start (colFilms.sub7, false);
   column->pack_start (colFilms.sub8, false);
   column->pack_start (colFilms.sub9, false);
   column->pack_start (colFilms.sub10, false);

   append_column (*Gtk::manage (column));
   column->set_resizable ();
   column->add_attribute (column->get_first_cell_renderer ()
			  ->property_visible(), colFilms.chgAll);

   set_rules_hint ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
FilmList::~FilmList () {
   TRACE9 ("FilmList::~FilmList ()");
}


//-----------------------------------------------------------------------------
/// Appends a film to the list
/// \param film: Film to add
/// \param director: Director of the film
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row FilmList::append (HFilm& film,
				       const Gtk::TreeModel::Row& director) {
   TRACE3 ("FilmList::append (HFilm&, Gtk::TreeModel::Row) - "
	   << (film ? film->getName ().c_str () : "None"));
   Check1 (film);

   HEntity obj (film);
   Gtk::TreeModel::Row newFilm (OwnerObjectList::append (obj, director));
   update (newFilm);
   return newFilm;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HFilm) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HFilm: Handle of the selected line
//-----------------------------------------------------------------------------
HFilm FilmList::getFilmAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HFilm film (boost::dynamic_pointer_cast<Film> (getObjectAt (iter))); Check3 (film);
   TRACE7 ("CDManager::getFilmAt (const Gtk::TreeIter&) - Selected film: " <<
	   film->getId () << '/' << film->getName ());
   return film;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void FilmList::setName (HEntity& object, const Glib::ustring& value) {
   (boost::dynamic_pointer_cast<Film> (object))->setName (value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
//-----------------------------------------------------------------------------
void FilmList::setYear (HEntity& object, const Glib::ustring& value) throw (std::exception) {
   (boost::dynamic_pointer_cast<Film> (object))->setYear (value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void FilmList::setGenre (HEntity& object, unsigned int value) {
   (boost::dynamic_pointer_cast<Film> (object))->setGenre (value);
}

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring FilmList::getColumnName () const {
   return _("Director/Film");
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int FilmList::sortEntity (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) const {
   HFilm ha (getFilmAt (a));
   HFilm hb (getFilmAt (b));
   int rc (Film::removeIgnored (ha->getName ()).compare (Film::removeIgnored (hb->getName ())));
   return rc ? rc : (ha->getName () < hb->getName ());
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void FilmList::valueChanged (const Glib::ustring& path,
			      const Glib::ustring& value, unsigned int column) {
   TRACE7 ("FilmList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);
   Check2 (column < 2);

   Gtk::TreeModel::Row row (*mOwnerObjects->get_iter (Gtk::TreeModel::Path (path)));

   try {
      if (row.parent ()) {
	 HFilm film (getFilmAt (row));
	 Glib::ustring oldValue;
	 switch (column) {
	 case 0: {
	    CDType& type (CDType::getInstance ());
	    if (!type.exists (value)) {
	       Glib::ustring e (_("Unknown type of media!"));
	       throw (YGP::InvalidValue (e));
	    }
	    film->setType (value);
	    oldValue = row[colFilms.type];
	    row[colFilms.type] = value;
	    break; }

	 case 1:
	    oldValue = film->getLanguage ();
	    setLanguage (row, value);
	    film->setLanguage (value);
	    break;

	 case 2:
	    oldValue = film->getTitles ();
	    setTitles (row, value);
	    film->setTitles (value);
	    break;

	 default:
	    Check3 (0);
	 } // end-switch

	 if (value != oldValue)
	    signalObjectChanged.emit (row, column + 3, oldValue);
      } // endif object edited
   } // end-try
   catch (std::exception& e) {
      YGP::StatusObject obj (YGP::StatusObject::ERROR, e.what ());
      obj.generalize (_("Invalid value!"));

      XGP::MessageDlg* dlg (XGP::MessageDlg::create (obj));
      dlg->set_title (PACKAGE);
      dlg->get_window ()->set_transient_for (this->get_window ());
   }
}

//-----------------------------------------------------------------------------
/// Callback for button-events in the listbox
/// \param e: Generated event
/// \returns bool: Whatever the default-method would return
//-----------------------------------------------------------------------------
bool FilmList::on_button_press_event (GdkEventButton* e) {
   TRACE9 ("FilmList::on_button_press_event (GdkEventButton*)");
   Check1 (e);

   Glib::RefPtr<Gtk::TreeSelection> selection (get_selection ());
   Gtk::TreeIter oldSel (selection->get_selected ());

   bool rc (OwnerObjectList::on_button_press_event (e));

   // Check if button 1 was pressed in the previously selected row
   if (((e->type == GDK_BUTTON_PRESS) && (e->button == 1))
       && selection->get_selected () && oldSel
       && (oldSel == selection->get_selected ())
       && (selection->get_selected ()->parent ())) {

      Gdk::Rectangle areaLang, areaSub;
      Check2 (get_column (4)); Check2 (get_column (5));
      Gtk::TreePath path (mOwnerObjects->get_path (oldSel));
      get_cell_area (path, *get_column (4), areaLang);
      get_cell_area (path, *get_column (5), areaSub);

      // If event is within the language or the subitles column
      if ((e->x > areaLang.get_x ())
	  && (e->x <= (areaSub.get_x () + areaSub.get_width ()))) {
	 TRACE9 ("FilmList::on_button_press_event (GdkEventButton*) - "
		 << areaLang.get_x () << '-' << e->x << '-'
		 << (areaSub.get_x () + areaSub.get_width ()));
	 // Create the popup-window
	 HFilm film (getFilmAt (oldSel)); Check3 (film);
	 if (e->x < areaSub.get_x ()) {
	    std::string languages (film->getLanguage ());
	    LanguageDialog dlg (languages, 5);
	    dlg.run ();

	    if (languages != film->getLanguage ())
	       valueChanged (path.to_string (), languages, 1);
	 }
	 else {
	    std::string titles (film->getTitles ());
	    LanguageDialog dlg (titles, 10, false);
	    dlg.set_title (_("Select subtitles"));
	    dlg.run ();

	    if (titles != film->getTitles ())
	       valueChanged (path.to_string (), titles, 2);
	 }

      }
   }
   return rc;
}

//-----------------------------------------------------------------------------
/// Sets the language-flags according to the passed value
/// \param row: Row to set the languages for
/// \param languages: Languages to show
//-----------------------------------------------------------------------------
void FilmList::setLanguage (Gtk::TreeModel::Row& row, const std::string& languages) {
   tokenizer langs (languages, boost::char_separator<char> (","));
   tokenizer::iterator l (langs.begin ());
   bool countSet (false);
   static const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >* columns[] =
      { &colFilms.lang1, &colFilms.lang2, &colFilms.lang3, &colFilms.lang4,
	&colFilms.lang5 };

   for (unsigned int i (0); i < (sizeof (columns) / sizeof (*columns)); ++i)
      if (l != langs.end ()) {
	 row[(*columns)[i]] = Language::findFlag (*l);
	 ++l;
      }
      else {
	 row[(*columns)[i]] = Glib::RefPtr<Gdk::Pixbuf> ();
	 if (!countSet) {
	    row[colFilms.langs] = i;
	    countSet = true;
	 }
      }
}

//-----------------------------------------------------------------------------
/// Sets the subtitle-flags according to the passed value
/// \param row: Row to set the subtitles for
/// \param titles: Subtitles to show
//-----------------------------------------------------------------------------
void FilmList::setTitles (Gtk::TreeModel::Row& row, const std::string& titles) {
   tokenizer langs (titles, boost::char_separator<char> (","));
   tokenizer::iterator l (langs.begin ());
   bool countSet (false);
   static const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >* columns[] =
      { &colFilms.sub1, &colFilms.sub2, &colFilms.sub3, &colFilms.sub4,
	&colFilms.sub5, &colFilms.sub6, &colFilms.sub7, &colFilms.sub8,
	&colFilms.sub9, &colFilms.sub10 };

   for (unsigned int i (0); i < (sizeof (columns) / sizeof (*columns)); ++i)
      if (l != langs.end ()) {
	 row[(*columns)[i]] = Language::findFlag (*l);
	 ++l;
      }
      else {
	 TRACE1 ("Set Title: " << titles << " = " << i);
	 row[(*columns)[i]] = Glib::RefPtr<Gdk::Pixbuf> ();
	 if (!countSet) {
	    row[colFilms.titles] = i;
	    countSet = true;
	 }
      }
}

//-----------------------------------------------------------------------------
/// Updates the displayed films, showing them in the passed language
/// \param lang: Language, in which the films should be displayed
//-----------------------------------------------------------------------------
void FilmList::update (const std::string& lang) {
   TRACE9 ("FilmList::update (const std::string&) - " << lang);

   for (Gtk::TreeModel::const_iterator d (mOwnerObjects->children ().begin ());
	d != mOwnerObjects->children ().end (); ++d) {
      for (Gtk::TreeIter m (d->children ().begin ()); m != d->children ().end (); ++m) {
	 HFilm film (getFilmAt (m)); Check3 (film);
	 TRACE9 ("FilmList::update (const std::string&) - Updating " << film->getName (lang));
	 (*m)[colOwnerObjects->name] = film->getName (lang);
      }
   }
}

//-----------------------------------------------------------------------------
/// Updates the displayed films; actualises the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void FilmList::update (Gtk::TreeModel::Row& row) {
   if (row->parent ()) {
      HFilm film (getFilmAt (row));
      row[colFilms.name] = film->getName ();
      row[colFilms.year] = film->getYear ().toString ();
      changeGenre (row, film->getGenre ());

      row[colFilms.type] = CDType::getInstance ()[film->getType ()];

      setLanguage (row, film->getLanguage ());
      setTitles (row, film->getTitles ());
   }
   OwnerObjectList::update (row);
}
