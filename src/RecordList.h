#ifndef RECORDLIST_H
#define RECORDLIST_H

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


#include <gtkmm/treeview.h>

#include "Record.h"
#include "Interpret.h"

#include "OOList.h"


class Genres;


/**Class to hold a list of records
 */
class RecordList : public OwnerObjectList {
 public:
   RecordList (const Genres& genres);
   virtual ~RecordList ();

   Gtk::TreeModel::Row insert (const HInterpret& artist, const Gtk::TreeIter& pos) {
      return OwnerObjectList::insert (artist, pos); }
   Gtk::TreeModel::Row append (const HInterpret& artist) { return insert (artist, mOwnerObjects->children ().end ()); }
   Gtk::TreeModel::Row prepend (const HInterpret& artist) { return insert (artist, mOwnerObjects->children ().begin ()); }

   Gtk::TreeModel::Row append (HRecord& record, const Gtk::TreeModel::Row& artist);

   HRecord getRecordAt (const Gtk::TreeIter iterator) const;
   HInterpret getInterpretAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }

   virtual void update (Gtk::TreeModel::Row& row);

 protected:
   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value) throw (std::exception);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const;

 private:
   RecordList (const RecordList& other);
   const RecordList& operator= (const RecordList& other);

   OwnerObjectColumns colOwnerObjects;
};


#endif
