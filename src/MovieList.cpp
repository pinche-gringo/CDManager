//$Id: MovieList.cpp,v 1.9 2004/12/07 03:34:40 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.9 $
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
#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "CDType.h"
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

   CellRendererList* const renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn
				      (_("Type"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text(), colMovies.type);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &MovieList::valueChanged), 0));

   CDType& type (CDType::getInstance ());
   for (CDType::const_iterator t (type.begin ()); t != type.end (); ++t)
      renderer->append_list_item (t->second);
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
   TRACE9 ("MovieList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);
   Check2 (column < 3);
   Check2 (colMovies);

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
