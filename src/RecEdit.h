#ifndef RECEDIT_H
#define RECEDIT_H

//$Id: RecEdit.h,v 1.8 2004/11/01 23:59:05 markus Rel $

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


#include <map>
#include <vector>

#include <gtkmm/liststore.h>

#include "Song.h"
#include "Record.h"
#include "Interpret.h"

#include <XGP/XDialog.h>
#include <XGP/XAttrEntry.h>


namespace Gtk {
   class Table;
   class Entry;
   class ComboBox;
}
class SongList;


class RecordEdit : public XGP::XDialog {
 public:
   RecordEdit (HRecord record, std::vector<HInterpret>& artists,
	       const std::map<unsigned int, Glib::ustring> genres);
   virtual ~RecordEdit ();

   static RecordEdit* create (HRecord record, std::vector<HInterpret>& artists,
			      const std::map<unsigned int, Glib::ustring> genres) {
      RecordEdit* dlg (new RecordEdit (record, artists, genres));
      dlg->signal_response ().connect (mem_fun (*dlg, &RecordEdit::free));
      return dlg;
   }

 protected:
   HRecord hRecord;

   virtual void okEvent ();

 private:
   //Prohibited manager functions
   RecordEdit (const RecordEdit& other);
   const RecordEdit& operator= (const RecordEdit& other);

   void fillGenres ();
   void fillInterprets ();

   Gtk::Table* pClient;

   XGP::XAttributeEntry<Glib::ustring>*                 txtRecord;
   XGP::XAttributeEntry<unsigned int, Gtk::SpinButton>* spinYear;
   Gtk::ComboBox*   optArtist;
   Gtk::ComboBox*   optGenre;
   SongList*        lstSongs;

   class ArtistColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      ArtistColumns () {
	 add (colID); add (colName); }

      Gtk::TreeModelColumn<unsigned long int> colID;
      Gtk::TreeModelColumn<Glib::ustring>     colName;
   };
   ArtistColumns colArtists;
   Glib::RefPtr<Gtk::ListStore> mArtists;

   class GenreColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      GenreColumns () {
	 add (colID); add (colName); }

      Gtk::TreeModelColumn<unsigned long int> colID;
      Gtk::TreeModelColumn<Glib::ustring>     colName;
   };
   GenreColumns colGenres;
   Glib::RefPtr<Gtk::ListStore> mGenres;

   std::vector<HInterpret>&                     artists;
   const std::map<unsigned int, Glib::ustring>& genres;
};


template <class T>
class TRecordEdit : public RecordEdit {
 public:
   typedef void (T::*PCALLBACK) (HRecord& hRecord);

   TRecordEdit (T& parent, PCALLBACK callback, HRecord record,
		std::vector<HInterpret>& artists,
		const std::map<unsigned int, Glib::ustring> genres)
      : RecordEdit (record, artists, genres), obj (parent),
	pCallback (callback) { }
   virtual ~TRecordEdit () { }

   static TRecordEdit* create (T& parent, PCALLBACK callback, HRecord record,
			       std::vector<HInterpret>& artists,
			       const std::map<unsigned int, Glib::ustring> genres) {
      TRecordEdit* dlg (new TRecordEdit<T> (parent, callback, record,
					    artists, genres));
      dlg->signal_response ().connect (mem_fun (*dlg, &TRecordEdit<T>::free));
      return dlg;
   }

 protected:
   virtual void okEvent () {
      RecordEdit::okEvent ();
      (obj.*pCallback) (hRecord); }

 private:
   //Prohibited manager functions
   TRecordEdit (const TRecordEdit& other);
   const TRecordEdit& operator= (const TRecordEdit& other);

   T&        obj;
   PCALLBACK pCallback;
};


#endif
