#ifndef RECORDLIST_H
#define RECORDLIST_H

//$Id: RecordList.h,v 1.8 2004/12/07 03:36:00 markus Rel $

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

#include "Record.h"
#include "Interpret.h"

#include "OOList.h"


class CellRendererList;


/**Class to hold a list of records
 */
class RecordList : public OwnerObjectList {
 public:
   RecordList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~RecordList ();

   Gtk::TreeModel::Row append (const HInterpret& artist) {
      return OwnerObjectList::append (artist); }
   Gtk::TreeModel::Row append (HRecord& record, const Gtk::TreeModel::Row& artist);

   HRecord getRecordAt (const Gtk::TreeIter iterator) const;
   HInterpret getInterpretAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }

 protected:
   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

 private:
   RecordList (const RecordList& other);
   const RecordList& operator= (const RecordList& other);

   OwnerObjectColumns colOwnerObjects;
};


#endif
