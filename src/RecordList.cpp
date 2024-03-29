//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009, 2010

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


#include <cdmgr-cfg.h>

#include <cerrno>
#include <cstdlib>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <XGP/XValue.h>

#include "Genres.h"

#include "RecordList.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
RecordList::RecordList (const Genres& genres)
   : OwnerObjectList (genres) {
   TRACE9 ("RecordList::RecordList (const Genres&)");
   mOwnerObjects = Gtk::TreeStore::create (colOwnerObjects);
   init (colOwnerObjects);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RecordList::~RecordList () {
   TRACE9 ("RecordList::~RecordList ()");
}


//-----------------------------------------------------------------------------
/// Appends a record to the list
/// \param record: Record to add
/// \param artist: Interpret of the record
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row RecordList::append (HRecord& record,
					const Gtk::TreeModel::Row& artist) {
   TRACE3 ("RecordList::append (HRecord&, Gtk::TreeModel::Row&) - "
	   << (record ? record->getName ().c_str () : "None"));
   Check1 (record);

   Gtk::TreeModel::Row newRecord (OwnerObjectList::append ((HEntity&)record, artist));
   update (newRecord);
   return newRecord;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HRecord) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HRecord: Handle of the selected line
//-----------------------------------------------------------------------------
HRecord RecordList::getRecordAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HRecord record (boost::dynamic_pointer_cast<Record> (getObjectAt (iter))); Check3 (record);
   TRACE7 ("RecordList::getRecordAt (const Gtk::TreeIter&) - Selected record: " <<
	   record->getId () << '/' << record->getName ());
   return record;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setName (HEntity& object, const Glib::ustring& value) {
   (boost::dynamic_pointer_cast<Record> (object))->setName (value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setYear (HEntity& object, const Glib::ustring& value) throw (std::exception) {
   (boost::dynamic_pointer_cast<Record> (object))->setYear (value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setGenre (HEntity& object, unsigned int value) {
   (boost::dynamic_pointer_cast<Record> (object))->setGenre (value);
}

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring RecordList::getColumnName () const {
   return _("Interpret/Record");
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int RecordList::sortEntity (const Gtk::TreeModel::iterator& a,
			    const Gtk::TreeModel::iterator& b) const {
   HRecord ha (getRecordAt (a));
   HRecord hb (getRecordAt (b));
   Glib::ustring aname (Record::removeIgnored (ha->getName ()));
   Glib::ustring bname (Record::removeIgnored (hb->getName ()));

   return ((aname < bname) ? -1 : (bname < aname) ? 1
	   : ha->getName ().compare (hb->getName ()));
}

//-----------------------------------------------------------------------------
/// Updates the displayed record; actualizes the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void RecordList::update (Gtk::TreeModel::Row& row) {
   if (row->parent ()) {
      HRecord record (getRecordAt (row));
      row[colOwnerObjects.name] = record->getName ();
      row[colOwnerObjects.year] = record->getYear ().toString ();
      changeGenre (row, record->getGenre ());
   }
   OwnerObjectList::update (row);
}
