#ifndef OOLIST_H
#define OOLIST_H

//$Id: OOList.h,v 1.15 2006/04/20 20:36:27 markus Rel $

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

#include "Genres.h"

#include "Celebrity.h"


typedef boost::shared_ptr<YGP::Entity> HEntity;


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
      add (entry); add (name); add (year); add (genre); add (chgAll); }

   Gtk::TreeModelColumn<HEntity>       entry;
   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
   Gtk::TreeModelColumn<bool>          chgAll;
};


/**Class to hold a list of movies
 */
class OwnerObjectList : public Gtk::TreeView {
 public:
   OwnerObjectList (const Genres& genres);
   virtual ~OwnerObjectList ();

   Gtk::TreeModel::Row insert (const HCelebrity& celebrity, const Gtk::TreeIter& pos);
   Gtk::TreeModel::Row append (const HCelebrity& celebrity) { return insert (celebrity, mOwnerObjects->children ().end ()); }
   Gtk::TreeModel::Row prepend (const HCelebrity& celebrity) { return insert (celebrity, mOwnerObjects->children ().begin ()); }

   Gtk::TreeModel::Row append (HEntity& object, const Gtk::TreeModel::Row& celebrity);
   void clear () { mOwnerObjects->clear (); }

   void updateGenres ();

   sigc::signal<void, const Gtk::TreeIter&, unsigned int, Glib::ustring&> signalOwnerChanged;
   sigc::signal<void, const Gtk::TreeIter&, unsigned int, Glib::ustring&> signalObjectChanged;

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mOwnerObjects; }
   HEntity getObjectAt (const Gtk::TreeIter iterator) const;
   HCelebrity getCelebrityAt (const Gtk::TreeIter iterator) const;

   Gtk::TreeModel::iterator getOwner (const Glib::ustring& name) const;
   Gtk::TreeModel::iterator getOwner (const HCelebrity& owner) const;
   Gtk::TreeModel::iterator getObject (const Gtk::TreeIter& parent,
				       const Glib::ustring& name) const;
   Gtk::TreeModel::iterator getObject (const Gtk::TreeIter& parent,
				       const HEntity& object) const;
   Gtk::TreeModel::iterator getObject (const HEntity& object) const;

   void selectRow (const Gtk::TreeModel::const_iterator& i);

   void set (Gtk::TreeModel::Row& row, const HEntity& obj);
   virtual void update (Gtk::TreeModel::Row& row);

   int getGenre (const Glib::ustring& genre) const { return genres.getId (genre); }

 protected:
   void init (const OwnerObjectColumns& cols);

   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value) throw (std::exception);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const = 0;

   void changeGenre (Gtk::TreeModel::Row& row, unsigned int value);

   int sortByName (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;
   int sortByYear (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;
   int sortByGenre (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;
   int sortOwner (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;

   const Genres& genres;
   const OwnerObjectColumns* colOwnerObjects;
   GenreColumns colGenres;

   Glib::RefPtr<Gtk::ListStore> mGenres;
   Glib::RefPtr<Gtk::TreeStore> mOwnerObjects;

 private:
   OwnerObjectList (const OwnerObjectList& other);
   const OwnerObjectList& operator= (const OwnerObjectList& other);
};


#endif
