#ifndef RECORDLIST_H
#define RECORDLIST_H

//$Id: RecordList.h,v 1.1 2004/11/07 02:33:21 markus Exp $

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

#include <Record.h>
#include <Interpret.h>


class CellRendererList;


/**Class describing the columns in the record-model
 */
class RecordColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   RecordColumns () {
      add (entry); add (name); add (year); add (genre); }

   Gtk::TreeModelColumn<HRecord>       entry;
   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<unsigned int>  year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class to hold a list of records
 */
class RecordList : public Gtk::TreeView {
 public:
   RecordList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~RecordList ();

   Gtk::TreeModel::Row append (const HInterpret& record);
   Gtk::TreeModel::Row append (HRecord& record, Gtk::TreeModel::Row artist);
   void clear () { mRecords->clear (); }

   void updateGenres ();

   sigc::signal<void, const HRecord&> signalChanged;

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mRecords; }
   HRecord getEntry (const Gtk::TreeIter iterator) const {
      return (*iterator)[colRecords.entry]; }

 protected:
   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

 private:
   RecordList (const RecordList& other);
   const RecordList& operator= (const RecordList& other);

   const std::map<unsigned int, Glib::ustring>& genres;

   RecordColumns colRecords;
   Glib::RefPtr<Gtk::TreeStore> mRecords;
};


#endif
