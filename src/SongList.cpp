//$Id: SongList.cpp,v 1.1 2004/11/01 23:58:20 markus Exp $

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

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/StatusObj.h>



//-----------------------------------------------------------------------------
/// Default constructor

//-----------------------------------------------------------------------------
SongList::SongList (const std::map<unsigned int, Glib::ustring>& genres)
   : genres (genres), mSongs (Gtk::ListStore::create (colSongs))
     , mSongGenres (Gtk::ListStore::create (colSongGenres)) {
   : genres (genres), mSongs (Gtk::ListStore::create (colSongs)) {
   TRACE9 ("SongList::SongList (const std::map<unsigned int, Glib::ustring>&");

   append_column (_("Track"), colSongs.colTrack);
   append_column (_("Song"), colSongs.colName);
#if 0
   Gtk::TreeViewColumn colTrack (_("Track"), colSongs.colTrack);
   colTrack.set_sort_indicator (true);
   colTrack.set_sort_order (Gtk::SORT_ASCENDING);
#endif

   unsigned int col;
   col = append_column (_("Track"), colSongs.colTrack) - 1;

   set_headers_clickable ();
   append_column (_("Genre"), colSongs.colGenre);

   for (unsigned int i (0); i < 4; ++i)
      get_column (col + i)->set_sort_column_id (i + 1);
      Gtk::TreeViewColumn* column (get_column (i));
   mSongs->set_sort_func (colSongs.colTrack,
			  bind (sigc::mem_fun (*this, &SongList::sort), 0));
   mSongs->set_sort_func (colSongs.colName,
			  bind (sigc::mem_fun (this, &SongList::sort), 1));
   mSongs->set_sort_func (colSongs.colDuration,
			  bind (sigc::mem_fun (this, &SongList::sort), 2));
   mSongs->set_sort_func (colSongs.colGenre,
			  bind (sigc::mem_fun (this, &SongList::sort), 3));
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
   set_headers_clickable ();
   mSongs->set_sort_column (colSongs.colTrack, Gtk::SORT_ASCENDING);
//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SongList::~SongList () {
   TRACE9 ("SongList::~SongList ()");
}

//-----------------------------------------------------------------------------
/// Appends a song to the list
/// \param song: Song to add
/// \returns Gtk::TreeModel::iterator: Inserted row
/// \param song: Song to add 
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->getName ().c_str () : "None"));
void SongList::append (HSong& song) {
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->name.c_str () : "None"));
   Gtk::TreeModel::Row newSong (*mSongs->append ());
   newSong[colSongs.entry] = song;
   newSong[colSongs.colTrack] = song->getTrack ();
   newSong[colSongs.colName] = song->getName ();
   newSong[colSongs.colTrack] = song->track;
   newSong[colSongs.colName] = song->name;
   newSong[colSongs.colDuration] = song->duration.toString ();
      (genres.find (song->getGenre ()));
   if (g == genres.end ())
      (genres.find (song->genre));
   newSong[colSongs.colGenre] = g->second;
   return newSong;
}
//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param a: Second entry to compare
/// \param entry: Flag which attribute of the song to use for the comparison
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int SongList::sort (const Gtk::TreeModel::iterator& a,
		    const Gtk::TreeModel::iterator& b, unsigned int entry) {
   TRACE9 ("SongList::sort (2x const Gtk::TreeModel::iterator&, unsigned int) - "
	   << entry);
   Check3 (entry < 4);

   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   HSong ha (rowa[colSongs.entry]);
   HSong hb (rowb[colSongs.entry]);

   switch (entry) {
   case 0:
      return ha->track - hb->track;

   case 1:
      return ha->name.compare (hb->name);

   case 2:
      return ha->duration.toSysTime () - hb->duration.toSysTime ();

   case 3:
      return ha->genre - hb->genre;
   break;
   } // endswitch
   return 0;
      TRACE1 ("Error: " << e.what ());
