//$Id: RecordList.cpp,v 1.8 2004/11/27 04:49:56 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.8 $
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Anticopyright (A) 2004

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


#include <cdmgr-cfg.h>

#include <cerrno>
#include <cstdlib>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <XGP/XValue.h>

#include "RendererList.h"

#include "RecordList.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
RecordList::RecordList (const std::map<unsigned int, Glib::ustring>& genres)
   : OwnerObjectList (genres) {
   TRACE9 ("RecordList::RecordList (const std::map<unsigned int, Glib::ustring>&)");
   init ();
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
   TRACE3 ("RecordList::append (HRecord&, Gtk::TreeModel::Row) - "
	   << (record.isDefined () ? record->name.c_str () : "None"));
   Check1 (record.isDefined ());

   HEntity obj (HEntity::cast (record));
   Gtk::TreeModel::Row newRecord (OwnerObjectList::append (obj, artist));
   newRecord[colOwnerObjects.name] = record->name;
   if (record->year)
      newRecord[colOwnerObjects.year] = record->year.toString ();
   changeGenre (newRecord, record->genre);

   return newRecord;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HRecord) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HRecord: Handle of the selected line
//-----------------------------------------------------------------------------
HRecord RecordList::getRecordAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HEntity hRec (getObjectAt (iter)); Check3 (hRec.isDefined ());
   HRecord record (HRecord::cast (hRec));
   TRACE7 ("CDManager::getRecordAt (const Gtk::TreeIter&) - Selected record: " <<
	   record->id << '/' << record->name);
   return record;
}


//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setName (HEntity& object, const Glib::ustring& value) {
   HRecord m (HRecord::cast (object));
   m->name = value;
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setYear (HEntity& object, const Glib::ustring& value) {
   HRecord m (HRecord::cast (object));
   m->year = value;
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void RecordList::setGenre (HEntity& object, unsigned int value) {
   HRecord m (HRecord::cast (object));
   m->genre = value;
}

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring RecordList::getColumnName () const {
   return _("Interpret/Record");
}
