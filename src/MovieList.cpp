//$Id: MovieList.cpp,v 1.12 2004/12/11 22:55:05 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.12 $
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (A) 2004

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

#include <cerrno>
#include <cstdlib>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Tokenize.h>
#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "CDType.h"
#include "LangDlg.h"
#include "RendererList.h"

#include "MovieList.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
MovieList::MovieList (const std::map<unsigned int, Glib::ustring>& genres)
   : OwnerObjectList (genres) {
   TRACE9 ("MovieList::MovieList (const std::map<unsigned int, Glib::ustring>&)");
   mOwnerObjects = Gtk::TreeStore::create (colMovies);
   init (colMovies);

   // Add column "Type"
   CellRendererList* renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn
				(_("Type"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text (), colMovies.type);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &MovieList::valueChanged), 0));

   CDType& type (CDType::getInstance ());
   for (CDType::const_iterator t (type.begin ()); t != type.end (); ++t)
      renderer->append_list_item (t->second);

   // Add column "Languages"
   column = new Gtk::TreeViewColumn (_("Language(s)"));
   column->pack_start (colMovies.lang1, false);
   column->pack_start (colMovies.lang2, false);
   column->pack_start (colMovies.lang3, false);
   column->pack_start (colMovies.lang4, false);
   column->pack_start (colMovies.lang5, false);

   append_column (*Gtk::manage (column));
   column->set_resizable ();
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
	   << (movie.isDefined () ? movie->getName ().c_str () : "None"));
   Check1 (movie.isDefined ());

   HEntity obj (HEntity::cast (movie));
   Gtk::TreeModel::Row newMovie (OwnerObjectList::append (obj, director));
   newMovie[colMovies.name] = movie->getName ();
   newMovie[colMovies.year] = movie->getYear ().toString ();
   changeGenre (newMovie, movie->getGenre ());

   newMovie[colMovies.type] = CDType::getInstance ()[movie->getType ()];

   std::string langs (movie->getLanguage ());
   movie->setLanguage (langs);
   setLanguage (newMovie, langs);
   return newMovie;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HMovie) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HMovie: Handle of the selected line
//-----------------------------------------------------------------------------
HMovie MovieList::getMovieAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HEntity hMovie (getObjectAt (iter)); Check3 (hMovie.isDefined ());
   HMovie movie (HMovie::cast (hMovie));
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
   HMovie m (HMovie::cast (object));
   m->setName (value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void MovieList::setYear (HEntity& object, const Glib::ustring& value) {
   HMovie m (HMovie::cast (object));
   m->setYear (value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void MovieList::setGenre (HEntity& object, unsigned int value) {
   HMovie m (HMovie::cast (object));
   m->setGenre (value);
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
			   const Gtk::TreeModel::iterator& b) {
   HMovie ha (getMovieAt (a));
   HMovie hb (getMovieAt (b));
   Glib::ustring aname (Movie::removeIgnored (ha->getName ()));
   Glib::ustring bname (Movie::removeIgnored (hb->getName ()));

   return ((aname < bname) ? -1 : (bname < aname) ? 1
	   : ha->getName ().compare (hb->getName ()));
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
	 signalObjectChanged.emit (getObjectAt (row));
	 switch (column) {
	 case 0: {
	    CDType& type (CDType::getInstance ());
	    if (!type.exists (value)) {
	       Glib::ustring e (_("Unknown type of media!"));
	       throw (std::runtime_error (e));
	    }
	    movie->setType (value);
	    row[colMovies.type] = value;
	    break; }

	 default:
	    Check3 (0);
	 } // end-switch
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
       && (oldSel == selection->get_selected ())
       && (selection->get_selected ()->parent ())) {
      unsigned int i (0);

      Gdk::Rectangle cellArea;
      Check3 (get_column (4));
      get_cell_area (mOwnerObjects->get_path (oldSel), *get_column (4), cellArea);

      // If event is within the language column
      if ((e->x > cellArea.get_x ())
	  && (e->x <= (cellArea.get_x () + cellArea.get_width ()))) {
	 TRACE9 ("MovieList::on_button_press_event (GdkEventButton*) - Column "
		 << i << ": " << cellArea.get_x () << '-' << e->x << '-'
		 << (cellArea.get_x () + cellArea.get_width ()));
	 // Create the popup-window
	 std::string languages (getMovieAt (oldSel)->getLanguage ());
	 LanguageDialog dlg (languages, 5);
	 dlg.run ();

	 Gtk::TreeRow row (*oldSel);
	 setLanguage (row, languages);
	 getMovieAt (oldSel)->setLanguage (languages);
      }
   }
   return rc;
}

//-----------------------------------------------------------------------------
/// Sets the language-flags according to the passed value
/// \param row: Row to set the languages for
/// \param languages: Languages to show; This string might be cut if it
///        exceeds the number of columns
//-----------------------------------------------------------------------------
void MovieList::setLanguage (Gtk::TreeModel::Row& row, const std::string& languages) {
   YGP::Tokenize langs (languages);
   bool countSet (false);
   static const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >* columns[] =
      { &colMovies.lang1, &colMovies.lang2, &colMovies.lang3, &colMovies.lang4,
	&colMovies.lang5 };

   for (unsigned int i (0); i < (sizeof (columns) / sizeof (*columns)); ++i)
      if (langs.getNextNode (',').size ())
	 row[(*columns)[i]] = Language::findFlag (langs.getActNode ());
      else {
	 row[(*columns)[i]] = Glib::RefPtr<Gdk::Pixbuf> ();
	 if (!countSet) {
	    row[colMovies.langs] = i;
	    countSet = true;
	 }
      }
}
