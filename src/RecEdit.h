#ifndef RECEDIT_H
#define RECEDIT_H

//$Id: RecEdit.h,v 1.4 2004/10/27 03:45:14 markus Exp $

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

#include <gtkmm/liststore.h>

#include "Song.h"
#include "Record.h"

#include <XGP/XDialog.h>

namespace Gtk {
   class Table;
   class Entry;
   class ComboBox;
   class TreeView;
   class SpinButton;
}


class RecordEdit : public XGP::XDialog {
 public:
   RecordEdit (HRecord record);
   virtual ~RecordEdit ();

   static RecordEdit* create (HRecord record) {
      RecordEdit* dlg (new RecordEdit (record));
      dlg->signal_response ().connect (mem_fun (*dlg, &RecordEdit::free));
      return dlg;
   }

 private:
   //Prohibited manager functions
   RecordEdit (const RecordEdit& other);
   const RecordEdit& operator= (const RecordEdit& other);

   void fillGenres ();
   void fillInterprets ();

   virtual void okEvent ();

   Gtk::Table* pClient;

   Gtk::Entry*      txtRecord;
   Gtk::SpinButton* spinYear;
   Gtk::ComboBox*   optArtist;
   Gtk::ComboBox*   optGenre;
   Gtk::TreeView*   lstSongs;

   HRecord hRecord;

   class ArtistColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      ArtistColumns () {
	 add (colID); add (colName); }

      Gtk::TreeModelColumn<int>           colID;
      Gtk::TreeModelColumn<Glib::ustring> colName;
   };
   ArtistColumns colArtists;
   Glib::RefPtr<Gtk::ListStore> mArtists;

   class GenreColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      GenreColumns () {
	 add (colID); add (colName); }

      Gtk::TreeModelColumn<int>           colID;
      Gtk::TreeModelColumn<Glib::ustring> colName;
   };
   GenreColumns colGenres;
   Glib::RefPtr<Gtk::ListStore> mGenres;

   class SongColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      SongColumns () {
	 add (entry); add (colName); add (colDuration); add (colGenre); }

      Gtk::TreeModelColumn<HSong>         entry;
      Gtk::TreeModelColumn<Glib::ustring> colName;
      Gtk::TreeModelColumn<Glib::ustring> colDuration;
      Gtk::TreeModelColumn<Glib::ustring> colGenre;
   };
   SongColumns colSongs;
   Glib::RefPtr<Gtk::ListStore> mSongs;

   std::map<unsigned int, Glib::ustring> genres;
};

#endif
