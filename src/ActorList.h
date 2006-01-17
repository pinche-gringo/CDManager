#ifndef ACTORLIST_H
#define ACTORLIST_H

//$Id: ActorList.h,v 1.1 2005/10/27 21:47:11 markus Rel $

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
   ActorList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~ActorList ();

   Gtk::TreeModel::Row append (const HActor& actor) {
      return OwnerObjectList::append (actor); }
   Gtk::TreeModel::Row append (HMovie& movie, const Gtk::TreeModel::Row& actor);

   HActor getActorAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }
   HMovie getMovieAt (const Gtk::TreeIter iterator) const;

   virtual void update (Gtk::TreeModel::Row& row);

 protected:
   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value);
   virtual void ActorList::setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

   static bool isParent (const Glib::RefPtr<Gtk::TreeModel>& model,
			 const Gtk::TreeModel::Path& path, bool);

 private:
   ActorList (const ActorList& other);
   const ActorList& operator= (const ActorList& other);

   OwnerObjectColumns colOwnerObjects;
};


#endif