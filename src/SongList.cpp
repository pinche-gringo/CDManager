//$Id: SongList.cpp,v 1.12 2004/12/22 17:00:08 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.12 $
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (A) 2004

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

#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "Words.h"

#include "SongList.h"
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
   column->add_attribute (renderer->property_text (), colSongs.colGenre);

      (bind (mem_fun (*this, &SongList::valueChanged), 3));

   mSongs->set_sort_column (colSongs.colTrack, Gtk::SORT_ASCENDING);
   mSongs->set_sort_func (colSongs.colTrack,
			  sigc::mem_fun (*this, &SongList::sortByTrack));
   mSongs->set_sort_func (colSongs.colName,
			  sigc::mem_fun (*this, &SongList::sortByName));

   set_search_column (colSongs.colName);
}

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
Gtk::TreeModel::iterator SongList::append (HSong& song) {
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->getName ().c_str () : "None"));
   Check1 (song.isDefined ());
   TRACE3 ("SongList::append (HSong&) - " << (song.isDefined () ? song->name.c_str () : "None"));
   Gtk::TreeModel::Row newSong (*mSongs->append ());
   newSong[colSongs.entry] = song;
   newSong[colSongs.colTrack] = song->getTrack ();
   newSong[colSongs.colName] = song->getName ();
   newSong[colSongs.colDuration] = song->getDuration ();

   std::map<unsigned int, Glib::ustring>::const_iterator g
      (genres.find (song->getGenre ()));
   if (g == genres.end ())
      g = genres.begin ();
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
	 if (track.isDefined ()) {
	    if (track == 0)
	       throw (std::runtime_error ("Invalid track number: `0'!"));

	    Gtk::TreeModel::const_iterator i (getSong (track));
	    if ((i != row) && (i != mSongs->children ().end ())) {
	       Glib::ustring e (_("Song `%1' already exists!"));
	       e.replace (e.find ("%1"), 2, value);
	       throw (std::runtime_error (e));
	    }
	 }
	 song->setTrack (track);
	 row[colSongs.colTrack] = song->getTrack ();
	 break; }
      case 1: {
	 Gtk::TreeModel::const_iterator i (getSong (value));
	 if ((i != row) && (i != mSongs->children ().end ())) {
	    Glib::ustring e (_("Song `%1' already exists!"));
	    e.replace (e.find ("%1"), 2, value);
	    throw (std::runtime_error (e));
	 }
	 song->setName (value);
	 row[colSongs.colName] = song->getName ();
	 break; }
      case 2:
	 song->setDuration (value);
	 row[colSongs.colDuration] = song->getDuration ();
	 break;
      case 3: {
	 for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	      g != genres.end (); ++g)
	    if (g->second == value) {
	       song->setGenre (g->first);
	       row[colSongs.colGenre] = value;
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
/// Sorts the entries in the song listbox
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int SongList::sortByTrack (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) {
   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   HSong ha (rowa[colSongs.entry]); Check3 (ha.isDefined ());
   HSong hb (rowb[colSongs.entry]); Check3 (hb.isDefined ());
   TRACE9 ("SongList::sortByTrack (2x const Gtk::TreeModel::iterator&) - "
	   << ha->getTrack () << '/' << hb->getTrack () << '='
	   << ha->getTrack ().compare (hb->getTrack ()));

   return ha->getTrack ().compare (hb->getTrack ());
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the song listbox
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int SongList::sortByName (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) {
   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   HSong ha (rowa[colSongs.entry]); Check3 (ha.isDefined ());
   HSong hb (rowb[colSongs.entry]); Check3 (hb.isDefined ());

   Glib::ustring aname (Words::removeArticle (ha->getName ()));
   Glib::ustring bname (Words::removeArticle (hb->getName ()));

   return ((aname < bname) ? -1 : (bname < aname) ? 1
	   : ha->getName ().compare (hb->getName ()));
}

//-----------------------------------------------------------------------------
/// Sets the genres list
//-----------------------------------------------------------------------------
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
      renderer->append_list_item (g->second);
//-----------------------------------------------------------------------------
/// Returns an iterator to the song having the passed value as name
/// \param name: Name of song
/// \returns Gtk::TreeModel::iterator: Iterator to found song or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator SongList::getSong (const Glib::ustring& name) {
   for (Gtk::TreeModel::const_iterator i (mSongs->children ().begin ());
	i != mSongs->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (actRow[colSongs.colName] == name)
	 return i;
   }
   return mSongs->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the song having the passed track number
/// \param track: Number of the song
/// \returns Gtk::TreeModel::iterator: Iterator to found song or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator SongList::getSong (const YGP::ANumeric& track) {
   for (Gtk::TreeModel::const_iterator i (mSongs->children ().begin ());
	i != mSongs->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      YGP::ANumeric value (actRow[colSongs.colTrack]);
      if (value == track)
	 return i;
   }
   return mSongs->children ().end ();
}
