//$Id: RecordList.cpp,v 1.1 2004/11/07 02:33:21 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
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

#define CHECK 9
#define TRACELEVEL 9
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
   : genres (genres), mRecords (Gtk::TreeStore::create (colRecords)) {
   TRACE9 ("RecordList::RecordList (const std::map<unsigned int, Glib::ustring>&)");

   set_model (mRecords);

   append_column (_("Interpret/Record"), colRecords.name);
   append_column (_("Year"), colRecords.year);

   set_headers_clickable ();

   for (unsigned int i (0); i < 2; ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
      column->set_sort_column_id (i + 1);
      column->set_resizable ();

      Check3 (get_column_cell_renderer (i));
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
      Check3 (typeid (*r) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (r));
      rText->property_editable () = true;
      rText->signal_edited ().connect
	 (bind (mem_fun (*this, &RecordList::valueChanged), i));
   }

   CellRendererList* const renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn
				      (_("Genre"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text(), colRecords.genre);

   column->set_sort_column_id (3);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &RecordList::valueChanged), 2));
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
Gtk::TreeModel::Row RecordList::append (HRecord& record, Gtk::TreeModel::Row artist) {
   TRACE3 ("RecordList::append (HRecord&, Gtk::TreeModel::Row) - "
	   << (record.isDefined () ? record->name.c_str () : "None"));
   Check1 (record.isDefined ());

   Gtk::TreeModel::Row newRecord (*mRecords->append (artist.children ()));
   newRecord[colRecords.entry] = record;
   newRecord[colRecords.name] = record->name;
   if (record->year)
      newRecord[colRecords.year] = record->year;

   std::map<unsigned int, Glib::ustring>::const_iterator g
      (genres.find (record->genre));
   if (g == genres.end ())
      g = genres.begin ();
   newRecord[colRecords.genre] = g->second;

   return newRecord;
}

//-----------------------------------------------------------------------------
/// Appends an interpret to the list
/// \param record: Record to add
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row RecordList::append (const HInterpret& artist) {
   TRACE3 ("RecordList::append (HInterpret&) - " << (artist.isDefined () ? artist->name.c_str () : "None"));
   Check1 (artist.isDefined ());

   Gtk::TreeModel::Row newArtist (*mRecords->append ());
   newArtist[colRecords.name] = artist->name;

   return newArtist;
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void RecordList::valueChanged (const Glib::ustring& path,
			       const Glib::ustring& value, unsigned int column) {
   TRACE9 ("RecordList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);

   Gtk::TreeModel::Row row (*mRecords->get_iter (Gtk::TreeModel::Path (path)));

   HRecord record (row[colRecords.entry]); Check3 (record.isDefined ());
   signalChanged.emit (record);
   try {
      switch (column) {
      case 0:
	 row[colRecords.name] = record->name= value;
	 break;
      case 1: {
	 char* pTail = NULL;
	 errno = 0;
	 unsigned int year (strtol (value.c_str (), &pTail, 0));
	 if (errno || (pTail && *pTail)) {
	    Glib::ustring e (_("No number: %1"));
	    e.replace (e.find ("%1"), 2, pTail);
	    throw std::invalid_argument (e.c_str ());
	 }
	 row[colRecords.year] = record->year = year;
	 break; }
      case 2: {
	 for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	      g != genres.end (); ++g)
	    if (g->second == value) {
	       record->genre = g->first;
	       row[colRecords.genre] = value;
	       return;
	    }
	 throw (std::invalid_argument (_("Unknown genre!")));
	 break; }
      default:
	 Check3 (0);
      } // endswitch
   }
   catch (std::exception& e) {
      YGP::StatusObject obj (YGP::StatusObject::ERROR, e.what ());
      obj.generalize (_("Invalid value!"));

      XGP::MessageDlg* dlg (XGP::MessageDlg::create (obj));
      dlg->set_title (PACKAGE);
      dlg->get_window ()->set_transient_for (this->get_window ());
   }
}

//-----------------------------------------------------------------------------
/// Sets the genres list
//-----------------------------------------------------------------------------
void RecordList::updateGenres () {
   TRACE9 ("RecordList::updateGenres () - Genres: " << genres.size ());

   Check3 (get_column_cell_renderer (2));
   Gtk::CellRenderer* r (get_column_cell_renderer (2)); Check3 (r);
   Check3 (typeid (*r) == typeid (CellRendererList));
   CellRendererList* renderer (dynamic_cast<CellRendererList*> (r));

   for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	g != genres.end (); ++g)
      renderer->append_list_item (g->second);
}
