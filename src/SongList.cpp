//$Id: SongList.cpp,v 1.2 2004/11/02 20:36:58 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
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
#include <YGP/StatusObj.h>


#include <XGP/XValue.h>


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
SongList::SongList (const std::map<unsigned int, Glib::ustring>& genres)
   : genres (genres), mSongs (Gtk::ListStore::create (colSongs))
     , mSongGenres (Gtk::ListStore::create (colSongGenres)) {
   : genres (genres), mSongs (Gtk::ListStore::create (colSongs)) {
   set_model (mSongs);

   append_column (_("Track"), colSongs.colTrack);
   append_column (_("Song"), colSongs.colName);
   append_column (_("Duration"), colSongs.colDuration);

   set_headers_clickable ();
   append_column (_("Genre"), colSongs.colGenre);

   for (unsigned int i (0); i < 3; ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
   for (unsigned int i (0); i < 4; ++i) {
      get_column (i)->set_sort_column_id (i + 1);
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
      Check3 (typeid (*r) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (r));
      rText->property_editable () = true;
      rText->signal_edited ().connect
	 (bind (mem_fun (*this, &SongList::valueChanged), i));
   }

   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn (_("Genre")));
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
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void SongList::valueChanged (const Glib::ustring& path,
			     const Glib::ustring& value, unsigned int column) {
   TRACE9 ("SongList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);

   Gtk::TreeModel::Row row (*mSongs->get_iter (Gtk::TreeModel::Path (path)));

   HSong song (row[colSongs.entry]); Check3 (song.isDefined ());
   row[colSongs.changed] = true;
   signalChanged.emit (song);
   try {
      case 0: {
	 YGP::ANumeric track (value);
      case 0:
	 row[colSongs.colTrack] = song->track = value;
	 break;

      case 1:
	 row[colSongs.colName] = song->name = value;
	 break;

	 row[colSongs.colDuration] = song->getDuration ();
	 row[colSongs.colDuration] = song->duration = value;
	 for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
   catch (std::exception& e) {
      YGP::StatusObject obj (YGP::StatusObject::ERROR, e.what ());
      obj.generalize (_("Invalid value!"));
      TRACE1 ("Error: " << e.what ());

