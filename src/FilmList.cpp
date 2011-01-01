//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009, 2010

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

#include "MovieList.h"


typedef boost::tokenizer<boost::char_separator<char> > tokenizer;


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
MovieList::MovieList (const Genres& genres)
   : OwnerObjectList (genres)
     , mTypes (Gtk::ListStore::create (colTypes)) {
   TRACE9 ("MovieList::MovieList (const Genres&)");
   mOwnerObjects = Gtk::TreeStore::create (colMovies);
   init (colMovies);

   // Add column "Type"
   Gtk::CellRendererCombo* renderer (new Gtk::CellRendererCombo ());
   renderer->property_text_column () = 0;
   renderer->property_model () = mTypes;
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn
				(_("Type"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text (), colMovies.type);
   column->add_attribute (renderer->property_visible(), colMovies.chgAll);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &MovieList::valueChanged), 0));

   CDType& type (CDType::getInstance ());
   for (CDType::const_iterator t (type.begin ()); t != type.end (); ++t) {
      Gtk::TreeModel::Row newType (*mTypes->append ());
      newType[colTypes.type] = (t->second);
   }

   // Add column "Languages"
   column = new Gtk::TreeViewColumn (_("Language(s)"));
   column->pack_start (colMovies.lang1, false);
   column->pack_start (colMovies.lang2, false);
   column->pack_start (colMovies.lang3, false);
   column->pack_start (colMovies.lang4, false);
   column->pack_start (colMovies.lang5, false);

   append_column (*Gtk::manage (column));
   column->set_resizable ();
   column->add_attribute (column->get_first_cell_renderer ()
			  ->property_visible(), colMovies.chgAll);

   // Add column "Subtitles"
   column = new Gtk::TreeViewColumn (_("Subtitles(s)"));
   column->pack_start (colMovies.sub1, false);
   column->pack_start (colMovies.sub2, false);
   column->pack_start (colMovies.sub3, false);
   column->pack_start (colMovies.sub4, false);
   column->pack_start (colMovies.sub5, false);
   column->pack_start (colMovies.sub6, false);
   column->pack_start (colMovies.sub7, false);
   column->pack_start (colMovies.sub8, false);
   column->pack_start (colMovies.sub9, false);
   column->pack_start (colMovies.sub10, false);

   append_column (*Gtk::manage (column));
   column->set_resizable ();
   column->add_attribute (column->get_first_cell_renderer ()
			  ->property_visible(), colMovies.chgAll);

   set_rules_hint ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MovieList::~MovieList () {
   TRACE9 ("MovieList::~MovieList ()");
}


//-----------------------------------------------------------------------------
/// Appends a movie to the list
/// \param movie: Movie to add
/// \param director: Director of the movie
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row MovieList::append (HMovie& movie,
				       const Gtk::TreeModel::Row& director) {
   TRACE3 ("MovieList::append (HMovie&, Gtk::TreeModel::Row) - "
	   << (movie ? movie->getName ().c_str () : "None"));
   Check1 (movie);

   HEntity obj (movie);
   Gtk::TreeModel::Row newMovie (OwnerObjectList::append (obj, director));
   update (newMovie);
   return newMovie;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HMovie) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HMovie: Handle of the selected line
//-----------------------------------------------------------------------------
HMovie MovieList::getMovieAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HMovie movie (boost::dynamic_pointer_cast<Movie> (getObjectAt (iter))); Check3 (movie);
   TRACE7 ("CDManager::getMovieAt (const Gtk::TreeIter&) - Selected movie: " <<
	   movie->getId () << '/' << movie->getName ());
   return movie;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void MovieList::setName (HEntity& object, const Glib::ustring& value) {
   (boost::dynamic_pointer_cast<Movie> (object))->setName (value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
//-----------------------------------------------------------------------------
void MovieList::setYear (HEntity& object, const Glib::ustring& value) throw (std::exception) {
   (boost::dynamic_pointer_cast<Movie> (object))->setYear (value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void MovieList::setGenre (HEntity& object, unsigned int value) {
   (boost::dynamic_pointer_cast<Movie> (object))->setGenre (value);
}

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring MovieList::getColumnName () const {
   return _("Director/Movie");
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int MovieList::sortEntity (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) const {
   HMovie ha (getMovieAt (a));
   HMovie hb (getMovieAt (b));
   int rc (Movie::removeIgnored (ha->getName ()).compare (Movie::removeIgnored (hb->getName ())));
   return rc ? rc : (ha->getName () < hb->getName ());
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void MovieList::valueChanged (const Glib::ustring& path,
			      const Glib::ustring& value, unsigned int column) {
   TRACE7 ("MovieList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);
   Check2 (column < 2);

   Gtk::TreeModel::Row row (*mOwnerObjects->get_iter (Gtk::TreeModel::Path (path)));

   try {
      if (row.parent ()) {
	 HMovie movie (getMovieAt (row));
	 Glib::ustring oldValue;
	 switch (column) {
	 case 0: {
	    CDType& type (CDType::getInstance ());
	    if (!type.exists (value)) {
	       Glib::ustring e (_("Unknown type of media!"));
	       throw (YGP::InvalidValue (e));
	    }
	    movie->setType (value);
	    oldValue = row[colMovies.type];
	    row[colMovies.type] = value;
	    break; }

	 case 1:
	    oldValue = movie->getLanguage ();
	    setLanguage (row, value);
	    movie->setLanguage (value);
	    break;

	 case 2:
	    oldValue = movie->getTitles ();
	    setTitles (row, value);
	    movie->setTitles (value);
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
bool MovieList::on_button_press_event (GdkEventButton* e) {
   TRACE9 ("MovieList::on_button_press_event (GdkEventButton*)");
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
	 TRACE9 ("MovieList::on_button_press_event (GdkEventButton*) - "
		 << areaLang.get_x () << '-' << e->x << '-'
		 << (areaSub.get_x () + areaSub.get_width ()));
	 // Create the popup-window
	 HMovie movie (getMovieAt (oldSel)); Check3 (movie);
	 if (e->x < areaSub.get_x ()) {
	    std::string languages (movie->getLanguage ());
	    LanguageDialog dlg (languages, 5);
	    dlg.run ();

	    if (languages != movie->getLanguage ())
	       valueChanged (path.to_string (), languages, 1);
	 }
	 else {
	    std::string titles (movie->getTitles ());
	    LanguageDialog dlg (titles, 10, false);
	    dlg.set_title (_("Select subtitles"));
	    dlg.run ();

	    if (titles != movie->getTitles ())
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
void MovieList::setLanguage (Gtk::TreeModel::Row& row, const std::string& languages) {
   tokenizer langs (languages, boost::char_separator<char> (","));
   tokenizer::iterator l (langs.begin ());
   bool countSet (false);
   static const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >* columns[] =
      { &colMovies.lang1, &colMovies.lang2, &colMovies.lang3, &colMovies.lang4,
	&colMovies.lang5 };

   for (unsigned int i (0); i < (sizeof (columns) / sizeof (*columns)); ++i)
      if (l != langs.end ()) {
	 row[(*columns)[i]] = Language::findFlag (*l);
	 ++l;
      }
      else {
	 row[(*columns)[i]] = Glib::RefPtr<Gdk::Pixbuf> ();
	 if (!countSet) {
	    row[colMovies.langs] = i;
	    countSet = true;
	 }
      }
}

//-----------------------------------------------------------------------------
/// Sets the subtitle-flags according to the passed value
/// \param row: Row to set the subtitles for
/// \param titles: Subtitles to show
//-----------------------------------------------------------------------------
void MovieList::setTitles (Gtk::TreeModel::Row& row, const std::string& titles) {
   tokenizer langs (titles, boost::char_separator<char> (","));
   tokenizer::iterator l (langs.begin ());
   bool countSet (false);
   static const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >* columns[] =
      { &colMovies.sub1, &colMovies.sub2, &colMovies.sub3, &colMovies.sub4,
	&colMovies.sub5, &colMovies.sub6, &colMovies.sub7, &colMovies.sub8,
	&colMovies.sub9, &colMovies.sub10 };

   for (unsigned int i (0); i < (sizeof (columns) / sizeof (*columns)); ++i)
      if (l != langs.end ()) {
	 row[(*columns)[i]] = Language::findFlag (*l);
	 ++l;
      }
      else {
	 TRACE1 ("Set Title: " << titles << " = " << i);
	 row[(*columns)[i]] = Glib::RefPtr<Gdk::Pixbuf> ();
	 if (!countSet) {
	    row[colMovies.titles] = i;
	    countSet = true;
	 }
      }
}

//-----------------------------------------------------------------------------
/// Updates the displayed movies, showing them in the passed language
/// \param lang: Language, in which the movies should be displayed
//-----------------------------------------------------------------------------
void MovieList::update (const std::string& lang) {
   TRACE9 ("MovieList::update (const std::string&) - " << lang);

   for (Gtk::TreeModel::const_iterator d (mOwnerObjects->children ().begin ());
	d != mOwnerObjects->children ().end (); ++d) {
      for (Gtk::TreeIter m (d->children ().begin ()); m != d->children ().end (); ++m) {
	 HMovie movie (getMovieAt (m)); Check3 (movie);
	 TRACE9 ("MovieList::update (const std::string&) - Updating " << movie->getName (lang));
	 (*m)[colOwnerObjects->name] = movie->getName (lang);
      }
   }
}

//-----------------------------------------------------------------------------
/// Updates the displayed movies; actualizes the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void MovieList::update (Gtk::TreeModel::Row& row) {
   if (row->parent ()) {
      HMovie movie (getMovieAt (row));
      row[colMovies.name] = movie->getName ();
      row[colMovies.year] = movie->getYear ().toString ();
      changeGenre (row, movie->getGenre ());

      row[colMovies.type] = CDType::getInstance ()[movie->getType ()];

      setLanguage (row, movie->getLanguage ());
      setTitles (row, movie->getTitles ());
   }
   OwnerObjectList::update (row);
}
