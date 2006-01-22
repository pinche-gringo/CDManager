//$Id: ActorList.cpp,v 1.2 2006/01/22 18:36:23 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Actor
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.09.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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

#include <XGP/XValue.h>

#include "ActorList.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
ActorList::ActorList (const Genres& genres)
   : OwnerObjectList (genres) {
   TRACE9 ("ActorList::ActorList (const Genres&)");
   mOwnerObjects = Gtk::TreeStore::create (colOwnerObjects);
   init (colOwnerObjects);

   Check3 (get_selection ());
   get_selection ()->set_select_function (sigc::ptr_fun (&ActorList::isParent));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ActorList::~ActorList () {
   TRACE9 ("ActorList::~ActorList ()");
}


//-----------------------------------------------------------------------------
/// Appends a actor to the list
/// \param actor: Movie to add
/// \param artist: Actor starring in the movie
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row ActorList::append (HMovie& movie,
				       const Gtk::TreeModel::Row& actor) {
   TRACE3 ("ActorList::append (HMovie&, Gtk::TreeModel::Row) - "
	   << (movie.isDefined () ? movie->getName ().c_str () : "None"));
   Check1 (movie.isDefined ());

   HEntity obj (HEntity::cast (movie));
   Gtk::TreeModel::Row newMovie (OwnerObjectList::append (obj, actor));
   update (newMovie);
   return newMovie;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HMovie) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HMovie: Handle of the selected line
//-----------------------------------------------------------------------------
HMovie ActorList::getMovieAt (const Gtk::TreeIter iter) const {
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
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void ActorList::setName (HEntity& object, const Glib::ustring& value) {
   HActor m (HActor::cast (object));
   m->setName (value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void ActorList::setYear (HEntity& object, const Glib::ustring& value) {
   HMovie m (HMovie::cast (object));
   m->setYear (value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void ActorList::setGenre (HEntity& object, unsigned int value) {
   HMovie m (HMovie::cast (object));
   m->setGenre (value);
}

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring ActorList::getColumnName () const {
   return _("Actors/Movies");
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortEntity (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) {
   HMovie ha (getMovieAt (a));
   HMovie hb (getMovieAt (b));
   int rc (Movie::removeIgnored (ha->getName ()).compare (Movie::removeIgnored (hb->getName ())));
   return rc ? rc : (ha->getName () < hb->getName ());
}

//-----------------------------------------------------------------------------
/// Checks if the passed path is a parent (first level node)
/// \param model: Model to check
/// \param path: Path to row to check
/// \returns bool: True, if row is a first level node
//-----------------------------------------------------------------------------
bool ActorList::isParent (const Glib::RefPtr<Gtk::TreeModel>& model,
			  const Gtk::TreeModel::Path& path, bool) {
   return !model->get_iter (path)->parent ();
}

//-----------------------------------------------------------------------------
/// Updates the displayed record; actualizes the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void ActorList::update (Gtk::TreeModel::Row& row) {
   if (row->parent ()) {
      HMovie movie (getMovieAt (row));
      row[colOwnerObjects.name] = movie->getName ();
      row[colOwnerObjects.year] = movie->getYear ().toString ();
      changeGenre (row, movie->getGenre ());
   }
   OwnerObjectList::update (row);
}
