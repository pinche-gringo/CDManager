#ifndef ACTORLIST_H
#define ACTORLIST_H

//$Id: ActorList.h,v 1.5 2006/04/18 20:44:07 markus Exp $

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

#include "Movie.h"
#include "Actor.h"

#include "OOList.h"


class CellRendererList;


/**Class to hold a list of actors (with the movies, they have played in)
 */
class ActorList : public OwnerObjectList {
 public:
   ActorList (const Genres& genres);
   virtual ~ActorList ();

   Gtk::TreeModel::Row append (const HActor& actor) {
      return OwnerObjectList::append (actor); }
   Gtk::TreeModel::Row append (HMovie& movie, const Gtk::TreeModel::Row& actor);

   HActor getActorAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }
   HMovie getMovieAt (const Gtk::TreeIter iterator) const;

   virtual void update (Gtk::TreeModel::Row& row);

 protected:
   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

 private:
   ActorList (const ActorList& other);
   const ActorList& operator= (const ActorList& other);

   OwnerObjectColumns colOwnerObjects;
};


#endif
