#ifndef SONGLIST_H
#define SONGLIST_H

//$Id: SongList.h,v 1.1 2004/11/01 23:58:20 markus Exp $

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

#include <Song.h>


/**Class describing the columns in the song-model
 */
class SongColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   SongColumns () {
      add (entry); add (colTrack); add (colName); add (colDuration);
      add (colGenre); }

   Gtk::TreeModelColumn<HSong>         entry;
   Gtk::TreeModelColumn<unsigned int>  colTrack;
   Gtk::TreeModelColumn<Glib::ustring> colName;
   Gtk::TreeModelColumn<Glib::ustring> colDuration;
   Gtk::TreeModelColumn<Glib::ustring> colGenre;
};


/**Class to hold a list of songs
 */
class SongList : public Gtk::TreeView {
 public:
   SongList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~SongList ();

   void append (HSong& song);
   void clear () { mSongs->clear (); }

 protected:   
   int sort (const Gtk::TreeModel::iterator& a,
	     const Gtk::TreeModel::iterator& b, unsigned int entry);

 private:
   SongList (const SongList& other);
   const SongList& operator= (const SongList& other);

   const std::map<unsigned int, Glib::ustring>& genres;

   SongColumns colSongs;
   Glib::RefPtr<Gtk::ListStore> mSongs;
};


#endif
