#ifndef SONGLIST_H
#define SONGLIST_H

//$Id: SongList.h,v 1.13 2006/01/28 03:50:55 markus Exp $

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

#include "Song.h"


class Genres;


/**Class describing the columns in the songgenre-model
 */
class SongGenreColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   SongGenreColumns () { add (genre); }

   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class describing the columns in the song-model
 */
class SongColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   SongColumns () {
      add (entry); add (colTrack); add (colName); add (colDuration);
      add (colGenre); }

   Gtk::TreeModelColumn<HSong>         entry;
   Gtk::TreeModelColumn<Glib::ustring> colTrack;
   Gtk::TreeModelColumn<Glib::ustring> colName;
   Gtk::TreeModelColumn<YGP::ATime>    colDuration;
   Gtk::TreeModelColumn<Glib::ustring> colGenre;
};


/**Class to hold a list of songs
 */
class SongList : public Gtk::TreeView {
 public:
   SongList (const Genres& genres);
   virtual ~SongList ();

   Gtk::TreeModel::iterator append (HSong& song);
   void clear () { mSongs->clear (); }

   void updateGenres ();
   void updateTrack (Gtk::TreeRow& row, const YGP::ANumeric& track) {
      row[colSongs.colTrack] = track.toString ();
      signalChanged.emit (row[colSongs.entry]); }
   void setGenre (Gtk::TreeIter& iter, unsigned int genre);

   virtual void update (Gtk::TreeModel::Row& row);

   Glib::RefPtr<Gtk::ListStore> getModel () const { return mSongs; }
   HSong getEntryAt (const Gtk::TreeModel::iterator& row) const {
      Gtk::TreeModel::Row r (*row);
      return r[colSongs.entry];
   }
   Gtk::TreeModel::iterator getSong (const HSong& song) const;
   Gtk::TreeModel::iterator getSong (const YGP::ANumeric& track) const;
   Gtk::TreeModel::iterator getSong (const Glib::ustring& name) const;

   sigc::signal<void, const HSong&> signalChanged;

 protected:
   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   int SongList::sortByTrack (const Gtk::TreeModel::iterator& a,
			      const Gtk::TreeModel::iterator& b);
   int SongList::sortByName (const Gtk::TreeModel::iterator& a,
			     const Gtk::TreeModel::iterator& b);

 private:
   SongList (const SongList& other);
   const SongList& operator= (const SongList& other);

   const Genres& genres;

   SongColumns colSongs;
   SongGenreColumns colSongGenres;
   Glib::RefPtr<Gtk::ListStore> mSongs;
   Glib::RefPtr<Gtk::ListStore> mSongGenres;
};


#endif
