#ifndef ACTORLIST_H
#define ACTORLIST_H

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


#include <YGP/Entity.h>

#include "Genres.h"

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>


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

/**Class to hold a list of actors (with the films, they have played in)
 */
class ActorList : public Gtk::TreeView {
 public:
   ActorList (const Genres& genres);
   virtual ~ActorList ();

   Gtk::TreeRow insert (const HEntity& entity, const Gtk::TreeModel::iterator& pos);
   Gtk::TreeRow append (const HEntity& entity) { return insert (entity, mOwnerObjects->children ().end ()); }
   Gtk::TreeRow prepend (const HEntity& entity) { return insert (entity, mOwnerObjects->children ().begin ()); }

   Gtk::TreeRow append (const HEntity& object, Gtk::TreeRow& owner);
   Gtk::TreeRow append (const HEntity& object, const Gtk::TreeModel::iterator& owner) {
      return append (object, *owner); }
   void clear () { mOwnerObjects->clear (); }

   void update (Gtk::TreeRow& row);

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mOwnerObjects; }

   /// Returns the handle at the passed position
   /// \param row: Row in the list
   /// \returns HEntity: Handle of the selected line
   HEntity getEntityAt (const Gtk::TreeModel::ConstRow& row) const {
      return row.get_value (colActors.entry);
   }
   /// Returns the handle at the passed position
   /// \param iter: Iterator to position in the list
   /// \returns HEntity: Handle of the selected line
   HEntity getEntityAt (const Gtk::TreeModel::const_iterator& iter) const {
      return getEntityAt (*iter);
   }

   Gtk::TreeModel::iterator findEntity (const HEntity& entry, unsigned int level,
					Gtk::TreeModel::iterator begin, Gtk::TreeModel::iterator end) const;
   Gtk::TreeModel::iterator findEntity (const HEntity& entry, unsigned int level = -1U) const {
      return findEntity (entry, level, mOwnerObjects->children ().begin (),
			 mOwnerObjects->children ().end ());
   }
   Gtk::TreeModel::iterator findName (const Glib::ustring& name,  unsigned int level = -1U) const {
      return findName (name, level, mOwnerObjects->children ().begin (),
		       mOwnerObjects->children ().end ());
   }
   Gtk::TreeModel::iterator findName (const Glib::ustring& name, unsigned int level,
				      Gtk::TreeModel::iterator begin, Gtk::TreeModel::iterator end) const;

   void selectRow (const Gtk::TreeModel::const_iterator& i);

   sigc::signal<void (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&)> signalActorChanged;

   ActorList (const ActorList& other) = delete;
   const ActorList& operator= (const ActorList& other) = delete;

 protected:
   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   int sortByName (const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const;
   int sortByYear (const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const;

 private:
   ActorColumns colActors;
   Glib::RefPtr<Gtk::TreeStore> mOwnerObjects;

   const Genres& genres;
};


#endif
