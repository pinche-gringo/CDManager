//$Id: SongList.cpp,v 1.5 2004/11/11 04:27:16 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.5 $
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

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>
#include "Words.h"

#include <XGP/XValue.h>

#include "RendererList.h"


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

   for (unsigned int i (0); i < 3; ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
      column->set_sort_column (i + 1);
      column->set_resizable ();
      column->set_sort_column_id (i + 1);
      Check3 (get_column_cell_renderer (i));
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
      Check3 (typeid (*r) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (r));
      rText->property_editable () = true;
      rText->signal_edited ().connect
	 (bind (mem_fun (*this, &SongList::valueChanged), i));
   }

   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn (_("Genre")));
   Gtk::CellRendererCombo* const renderer (new Gtk::CellRendererCombo);
   CellRendererList*    const renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* const column   (new Gtk::TreeViewColumn
					(_("Genre"), *Gtk::manage (renderer)));
   column->set_sort_column (4);
   column->add_attribute (renderer->property_text(), colSongs.colGenre);
   column->set_resizable ();
   column->set_sort_column_id (4);

      (bind (mem_fun (*this, &SongList::valueChanged), 3));

   mSongs->set_sort_column (colSongs.colTrack, Gtk::SORT_ASCENDING);
   mSongs->set_sort_func (colSongs.colTrack,
   mSongs->set_sort_func (colSongs.colName,
			  sigc::mem_fun (*this, &SongList::sortByName));
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
//-----------------------------------------------------------------------------
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->getName ().c_str () : "None"));
void SongList::append (HSong& song) {
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->name.c_str () : "None"));
   Gtk::TreeModel::Row newSong (*mSongs->append ());
   newSong[colSongs.entry] = song;
   newSong[colSongs.colTrack] = song->getTrack ();
   newSong[colSongs.colName] = song->getName ();
   newSong[colSongs.colTrack] = song->track;
   newSong[colSongs.colName] = song->name;
   newSong[colSongs.colDuration] = song->duration;
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
   signalChanged.emit (song);
   try {
      switch (column) {
      case 0: {
	 YGP::ANumeric track (value);
      case 0:
	 row[colSongs.colTrack] = song->track = value;
	 break;
      case 1:
	 row[colSongs.colName] = song->name= value;
	 break;
	 row[colSongs.colDuration] = song->getDuration ();
	 row[colSongs.colDuration] = song->duration = value;
	 for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	      g != genres.end (); ++g)
	    if (g->second == value) {
	       song->setGenre (g->first);
	       row[colSongs.colGenre] = value;
	       song->genre = g->first;
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
/// Sorts the entries in the song listbox
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
/// \param entry: Flag which attribute of the song to use for the comparison
int SongList::sortByTrack (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) {
   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   TRACE9 ("SongList::sortByTrack (2x const Gtk::TreeModel::iterator&)");

   HSong ha (rowa[colSongs.entry]); Check3 (ha.isDefined ());
   HSong hb (rowb[colSongs.entry]); Check3 (hb.isDefined ());
   HSong ha (rowa[colSongs.entry]);
   HSong hb (rowb[colSongs.entry]);
}
   return ha->track - hb->track;
//-----------------------------------------------------------------------------
/// Sorts the entries in the song listbox
/// \param a: First entry to compare
void SongList::updateGenres () {
   TRACE9 ("SongList::updateGenres () - Genres: " << genres.size ());

   mSongGenres->clear ();
   for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
   Check3 (get_column_cell_renderer (3));
   Gtk::CellRenderer* r (get_column_cell_renderer (3)); Check3 (r);
   Check3 (typeid (*r) == typeid (CellRendererList));
   CellRendererList* renderer (dynamic_cast<CellRendererList*> (r));

      Gtk::TreeModel::Row newGenre (*mSongGenres->append ());
	g != genres.end (); ++g)
