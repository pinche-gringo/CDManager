#ifndef MOVIELIST_H
#define MOVIELIST_H

//$Id: MovieList.h,v 1.5 2004/11/29 19:03:24 markus Rel $

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


#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

#include "Movie.h"
#include "Director.h"

#include "OOList.h"


class CellRendererList;


/**Class to hold a list of movies
 */
class MovieList : public OwnerObjectList {
 public:
   MovieList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~MovieList ();

   Gtk::TreeModel::Row append (const HDirector& director) {
      return OwnerObjectList::append (director); }
   Gtk::TreeModel::Row append (HMovie& movie, const Gtk::TreeModel::Row& director);

   HMovie getMovieAt (const Gtk::TreeIter iterator) const;
   HDirector getDirectorAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }

 protected:
   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

 private:
   MovieList (const MovieList& other);
   const MovieList& operator= (const MovieList& other);
};


#endif
