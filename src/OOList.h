#ifndef OOLIST_H
#define OOLIST_H

//$Id: OOList.h,v 1.11 2006/01/22 18:36:23 markus Exp $

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
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>

#include "Celebrity.h"


class Genres;

typedef YGP::Handle<YGP::Entity> HEntity;


/**Class describing the columns in the genre-model
 */
class GenreColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   GenreColumns () { add (genre); }

   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class describing the columns in the Object-Owner list
 */
class OwnerObjectColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   OwnerObjectColumns () {
      add (entry); add (name); add (year); add (genre); }

   Gtk::TreeModelColumn<HEntity>       entry;
   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class to hold a list of movies
 */
class OwnerObjectList : public Gtk::TreeView {
 public:
   OwnerObjectList (const Genres& genres);
   virtual ~OwnerObjectList ();

   Gtk::TreeModel::Row append (const HCelebrity& celebrity);
   virtual Gtk::TreeModel::Row append (HEntity& movie, const Gtk::TreeModel::Row& celebrity);
   void clear () { mOwnerObjects->clear (); }

   void updateGenres ();

   sigc::signal<void, const HCelebrity&> signalOwnerChanged;
   sigc::signal<void, const HEntity&> signalObjectChanged;
   sigc::signal<void, const HEntity&> signalObjectGenreChanged;

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mOwnerObjects; }
   HEntity getObjectAt (const Gtk::TreeIter iterator) const;
   HCelebrity getCelebrityAt (const Gtk::TreeIter iterator) const;

   Gtk::TreeModel::iterator getOwner (const Glib::ustring& name);
   Gtk::TreeModel::iterator getOwner (const HCelebrity& owner);
   Gtk::TreeModel::iterator getObject (const Gtk::TreeIter& parent,
				       const Glib::ustring& name);
   Gtk::TreeModel::iterator getObject (const Gtk::TreeIter& parent,
				       const HEntity& object);
   Gtk::TreeModel::iterator getObject (const HEntity& object);

   void selectRow (const Gtk::TreeModel::const_iterator& i);

   void set (Gtk::TreeModel::Row& row, const HEntity& obj);
   virtual void update (Gtk::TreeModel::Row& row);

 protected:
   void init (const OwnerObjectColumns& cols);

   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const = 0;

   void changeGenre (Gtk::TreeModel::Row& row, unsigned int value);

   int sortByName (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
   int sortByYear (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
   int sortByGenre (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
   int sortOwner (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

   const Genres& genres;
   const OwnerObjectColumns* colOwnerObjects;
   GenreColumns colGenres;

   Glib::RefPtr<Gtk::ListStore> mGenres;
   Glib::RefPtr<Gtk::TreeStore> mOwnerObjects;

 private:
   OwnerObjectList (const OwnerObjectList& other);
   const OwnerObjectList& operator= (const OwnerObjectList& other);

   Glib::ustring getLiveSpan (const HCelebrity& owner);
};


#endif
