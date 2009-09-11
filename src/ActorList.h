#ifndef ACTORLIST_H
#define ACTORLIST_H

//$Id: ActorList.h,v 1.7 2006/04/24 02:16:44 markus Rel $

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


#include <YGP/Entity.h>

#include "Genres.h"

#include "gtkmm/treeview.h"
#include "gtkmm/treestore.h"


/**Class describing the columns in the Object-Owner list
 */
class ActorColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   ActorColumns () {
      add (entry); add (name); add (year); add (genre); add (editable); }

   Gtk::TreeModelColumn<HEntity>       entry;
   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
   Gtk::TreeModelColumn<bool>          editable;
};

/**Class to hold a list of actors (with the movies, they have played in)
 */
class ActorList : public Gtk::TreeView {
 public:
   ActorList (const Genres& genres);
   virtual ~ActorList ();

   Gtk::TreeRow insert (const HEntity& entity, const Gtk::TreeIter& pos);
   Gtk::TreeRow append (const HEntity& entity) { return insert (entity, mOwnerObjects->children ().end ()); }
   Gtk::TreeRow prepend (const HEntity& entity) { return insert (entity, mOwnerObjects->children ().begin ()); }

   Gtk::TreeRow append (const HEntity& object, const Gtk::TreeIter& owner);
   void clear () { mOwnerObjects->clear (); }

   void update (Gtk::TreeRow& row);

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mOwnerObjects; }

   /// Returns the handle at the passed position
   /// \param iter: Iterator to position in the list
   /// \returns HMovie: Handle of the selected line
   HEntity getEntityAt (const Gtk::TreeIter iter) const {
      HEntity hEntity ((*iter)[colActors.entry]);
      return hEntity;
   }

   Gtk::TreeIter findEntity (const HEntity& entry, unsigned int level,
			     Gtk::TreeIter begin, Gtk::TreeIter end) const;
   Gtk::TreeIter findEntity (const HEntity& entry, unsigned int level = -1U) const {
      return findEntity (entry, level, mOwnerObjects->children ().begin (),
			 mOwnerObjects->children ().end ());
   }
   Gtk::TreeIter findName (const Glib::ustring& name,  unsigned int level = -1U) const {
      return findName (name, level, mOwnerObjects->children ().begin (),
		       mOwnerObjects->children ().end ());
   }
   Gtk::TreeIter findName (const Glib::ustring& name, unsigned int level,
			   Gtk::TreeIter begin, Gtk::TreeIter end) const;

   void selectRow (const Gtk::TreeModel::const_iterator& i);

   sigc::signal<void, const Gtk::TreeIter&, unsigned int, Glib::ustring&> signalActorChanged;

 protected:
   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   int sortByName (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;
   int sortByYear (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;

 private:
   ActorList (const ActorList& other);
   const ActorList& operator= (const ActorList& other);

   ActorColumns colActors;
   Glib::RefPtr<Gtk::TreeStore> mOwnerObjects;

   const Genres& genres;
};


#endif
