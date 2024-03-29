//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2009 - 2011

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.


#include <cdmgr-cfg.h>

#include <gtkmm/cellrenderercombo.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "Words.h"

#include "SongList.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
SongList::SongList (const Genres& genres)
   : genres (genres), mSongs (Gtk::ListStore::create (colSongs)),
     mSongGenres (Gtk::ListStore::create (colSongGenres)) {
   TRACE9 ("SongList::SongList (const Genres&) - " << genres.size ());
   Check3 (genres.size ());

   set_model (mSongs);
   updateGenres ();

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
   column->pack_start (*manage (renderer));
   append_column (*Gtk::manage (column));

   column->set_sort_column (colSongs.colGenre);
   column->set_resizable ();
   column->add_attribute (renderer->property_text (), colSongs.colGenre);

   renderer->property_text_column () = 0;
   renderer->property_model () = mSongGenres;
   renderer->property_editable () = true;

   renderer->signal_edited ().connect
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
/// Inserts a song into the list
/// \param song: Song to add
/// \param pos: Position in model for insert
/// \returns Gtk::TreeModel::iterator: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator SongList::insert (HSong& song, const Gtk::TreeIter& pos) {
   TRACE3 ("SongList::insert (HSong&, const TreeIter&) - " << (song ? song->getName ().c_str () : "None"));
   Check1 (song);

   Gtk::TreeModel::Row newSong (*mSongs->insert (pos));
   newSong[colSongs.entry] = song;
   update (newSong);
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

   Glib::ustring oldValue;
   HSong song (row[colSongs.entry]); Check3 (song);
   try {
      switch (column) {
      case 0: {
	 YGP::ANumeric track (value);
	 if (track.isDefined ()) {
	    if (track == 0)
	       throw (YGP::InvalidValue ("Invalid track number: `0'!"));

	    Gtk::TreeModel::const_iterator i (getSong (track));
	    if ((i != row) && (i != mSongs->children ().end ())) {
	       Glib::ustring e (_("Song `%1' already exists!"));
	       e.replace (e.find ("%1"), 2, value);
	       throw (YGP::InvalidValue (e));
	    }
	 }
	 song->setTrack (track);
	 oldValue = row[colSongs.colTrack];
	 row[colSongs.colTrack] = value;
	 break; }

      case 1: {
	 Gtk::TreeModel::const_iterator i (getSong (value));
	 if ((i != row) && (i != mSongs->children ().end ())) {
	    Glib::ustring e (_("Song `%1' already exists!"));
	    e.replace (e.find ("%1"), 2, value);
	    throw (YGP::InvalidValue (e));
	 }
	 song->setName (value);
	 oldValue = row[colSongs.colName];
	 row[colSongs.colName] = song->getName ();
	 break; }

      case 2:
	 song->setDuration (value);
	 oldValue = YGP::ATime (row[colSongs.colDuration]).toUnformattedString ();
	 row[colSongs.colDuration] = song->getDuration ();
	 break;

      case 3: {
	 int g (genres.getId (value));
	 if (g != -1) {
	    oldValue = Glib::ustring (1, (char)song->getGenre ());
	    song->setGenre (g);
	    row[colSongs.colGenre] = value;
	    break;
	 }
	 else
	    throw (YGP::InvalidValue (_("Unknown genre!")));
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
   signalChanged.emit (row, column, oldValue);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the song listbox
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value of compare (analogue to strcmp)
//-----------------------------------------------------------------------------
int SongList::sortByTrack (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) const {
   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   HSong ha (rowa[colSongs.entry]); Check3 (ha);
   HSong hb (rowb[colSongs.entry]); Check3 (hb);
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
			   const Gtk::TreeModel::iterator& b) const {
   Gtk::TreeModel::Row rowa (*a);
   Gtk::TreeModel::Row rowb (*b);
   HSong ha (rowa[colSongs.entry]); Check3 (ha);
   HSong hb (rowb[colSongs.entry]); Check3 (hb);

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
   for (unsigned int i (0); i < genres.size (); ++i) {
      Gtk::TreeModel::Row newGenre (*mSongGenres->append ());
      newGenre[colSongGenres.genre] = (genres.getGenre (i));
   }
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the song identified by the passed handle
/// \param song: Handle of the song
/// \returns Gtk::TreeModel::iterator: Iterator to found song or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator SongList::getSong (const HSong& song) const {
   for (Gtk::TreeModel::const_iterator i (mSongs->children ().begin ());
	i != mSongs->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (song == (HSong)actRow[colSongs.entry])
	 return i;
   }
   return mSongs->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the song having the passed value as name
/// \param name: Name of song
/// \returns Gtk::TreeModel::iterator: Iterator to found song or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator SongList::getSong (const Glib::ustring& name) const {
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
Gtk::TreeModel::iterator SongList::getSong (const YGP::ANumeric& track) const {
   Glib::ustring strTrack (track.toString ());
   for (Gtk::TreeModel::const_iterator i (mSongs->children ().begin ());
	i != mSongs->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      Glib::ustring value (actRow[colSongs.colTrack]);
      if (value == strTrack)
	 return i;
   }
   return mSongs->children ().end ();
}

//-----------------------------------------------------------------------------
/// Sets the genre of a song in the entity (if not already set) and the list
/// \param iter: Iterator to song to change
/// \param genre: Genre to set
//-----------------------------------------------------------------------------
void SongList::setGenre (Gtk::TreeIter& iter, unsigned int genre) {
   TRACE9 ("SongList::setGenre (Gtk::TreeIter&, unsigned int) - " << genre);
   Check1 (iter);
   HSong song (getSongAt (iter)); Check2 (song);
   TRACE9 ("SongList::setGenre (Gtk::TreeIter&, unsigned int) - Genre: " << song->getGenre ());
   if (!song->getGenre ()) {
      TRACE9 ("SongList::setGenre (Gtk::TreeIter&, unsigned int) - Changing " << song->getName ());
      Glib::ustring oldValue (1, song->getGenre ());

      song->setGenre (genre);
      signalChanged.emit (iter, 3, oldValue);

      if (genre >= genres.size ())
	  genre = 0;
      (*iter)[colSongs.colGenre] = genres.getGenre (genre);
   }
}

//-----------------------------------------------------------------------------
/// Updates the displayed songs; actualises the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void SongList::update (Gtk::TreeModel::Row& row) {
   TRACE9 ("SongList::update (Gtk::TreeModel::Row&)");

   HSong song (row[colSongs.entry]); Check3 (song);
   row[colSongs.colTrack] = song->getTrack ().toString ();
   row[colSongs.colName] = song->getName ();
   row[colSongs.colDuration] = song->getDuration ();

   unsigned int genre (song->getGenre ());
   if (genre >= genres.size ())
      genre = 0;
   row[colSongs.colGenre] = genres.getGenre (genre);
}
