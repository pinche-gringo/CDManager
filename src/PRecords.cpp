//$Id: PRecords.cpp,v 1.20 2009/08/08 13:24:28 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Records
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.20 $
//AUTHOR      : Markus Schwab
//CREATED     : 24.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2007, 2009, 2010

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

#include <unistd.h>

#include <fstream>
#include <sstream>

#include <gtkmm/paned.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>

#include <XGP/XFileDlg.h>
#include <XGP/MessageDlg.h>

#include "SaveCeleb.h"
#include "StorageRecord.h"

#include "PRecords.h"


//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling records/songs
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
//-----------------------------------------------------------------------------
PRecords::PRecords (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres)
   : NBPage (status, menuSave), records (genres), songs (genres),
     relRecords ("records"), relSongs ("songs") {
   TRACE9 ("PRecords::PRecords (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&)");

   Gtk::HPaned* cds (new Gtk::HPaned);
   Gtk::ScrolledWindow* scrlRecords (new Gtk::ScrolledWindow);
   Gtk::ScrolledWindow* scrlSongs (new Gtk::ScrolledWindow);

   scrlRecords->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlSongs->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlRecords->add (records);
   scrlSongs->add (songs);
   scrlRecords->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlSongs->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   Glib::RefPtr<Gtk::TreeSelection> sel (songs.get_selection ());
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &PRecords::songSelected));
   songs.signalChanged.connect (mem_fun (*this, &PRecords::songChanged));

   records.signalOwnerChanged.connect (mem_fun (*this, &PRecords::interpretChanged));
   records.signalObjectChanged.connect (mem_fun (*this, &PRecords::recordChanged));

   sel = records.get_selection ();
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &PRecords::recordSelected));

   cds->add1 (*manage (scrlRecords));
   cds->add2 (*manage (scrlSongs));
   cds->set_position (400);

   widget = cds;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PRecords::~PRecords () {
   TRACE9 ("PRecords::~PRecords ()");
}


//-----------------------------------------------------------------------------
/// Loads the records from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void PRecords::loadData () {
   TRACE9 ("PRecords::loadData ()");
   try {
      YGP::StatusObject status;
      StorageRecord::loadInterprets (interprets, status);
      std::sort (interprets.begin (), interprets.end (), &Interpret::compByName);

      std::map<unsigned int, std::vector<HRecord> > aRecords;
      unsigned int cRecords (StorageRecord::loadRecords (aRecords, status));
      TRACE8 ("PRecords::loadData () - Found " << aRecords.size () << " records");

      for (std::vector<HInterpret>::const_iterator i (interprets.begin ());
	   i != interprets.end (); ++i) {
	 Gtk::TreeModel::Row interpret (records.append (*i));

	 std::map<unsigned int, std::vector<HRecord> >::iterator iRec
	    (aRecords.find ((*i)->getId ()));
	 if (iRec != aRecords.end ()) {
	    for (std::vector<HRecord>::iterator r (iRec->second.begin ());
		 r != iRec->second.end (); ++r) {
	       records.append (*r, interpret);
	       relRecords.relate (*i, *r);
	    }
	    aRecords.erase (iRec);
	 } // end-if artist has record
      } // end-for all artists
      records.expand_all ();

      loaded = true;

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 record", "Loaded %1 records", cRecords)));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cRecords));

      Glib::ustring tmp (Glib::locale_to_utf8 (ngettext (" from %1 artist", " from %1 artists", interprets.size ())));
      tmp.replace (tmp.find ("%1"), 2, YGP::ANumeric::toString (interprets.size ()));
      msg += tmp;
      showStatus (msg);

      if (status.getType () > YGP::StatusObject::UNDEFINED) {
	 status.generalize (_("Warnings loading records"));
	 XGP::MessageDlg::create (status);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available records!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the songs for the passed record
/// \param record: Handle to the record for which to load songs
//-----------------------------------------------------------------------------
void PRecords::loadSongs (const HRecord& record) {
   TRACE9 ("PRecords::loadSongs (const HRecord& record) - "
	   << (record ? record->getName ().c_str () : "Undefined"));
   Check1 (record);

   try {
      std::vector<HSong> songs_;
      StorageRecord::loadSongs (record->getId (), songs_);
      TRACE5 ("PRecords::loadSongs (const HRecord& record) - Found songs: " << songs_.size ());

      if (songs_.size ())
	  relSongs.relate (record, songs_);
      record->setSongsLoaded ();
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the songs for record %1!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, record->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Adds a new interpret to the list
//-----------------------------------------------------------------------------
void PRecords::newInterpret () {
   HInterpret interpret (new Interpret);
   addInterpret (interpret);
}

//-----------------------------------------------------------------------------
/// Adds a new record to the first selected interpret
//-----------------------------------------------------------------------------
void PRecords::newRecord () {
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
   Check3 (list.size ());
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   Gtk::TreeIter p (model->get_iter (*list.begin ())); Check3 (p);
   if ((*p)->parent ())
      p = ((*p)->parent ());

   HRecord record (new Record);
   record->setSongsLoaded ();
   addRecord (p, record);
}

//-----------------------------------------------------------------------------
/// Adds a new song to the first selected record
//-----------------------------------------------------------------------------
void PRecords::newSong () {
   HSong song (new Song);
   addSong (song);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a record
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PRecords::recordSelected () {
   TRACE9 ("PRecords::recordSelected ()");
   songs.clear ();
   Check3 (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list
      (records.get_selection ()->get_selected_rows ());
   TRACE9 ("PRecords::recordSelected () - Size: " << list.size ());
   if (list.size ()) {
      Gtk::TreeIter i (records.get_model ()->get_iter (*list.begin ())); Check3 (i);

      if ((*i)->parent ()) {
	 HRecord hRecord (records.getRecordAt (i)); Check3 (hRecord);
	 if (hRecord->needsLoading () && hRecord->getId ())
	    loadSongs (hRecord);
	 Check3 (!hRecord->needsLoading ());

	 // Add related songs to the listbox
	 if (relSongs.isRelated (hRecord)) {
	    const std::vector<HSong>& as (relSongs.getObjects (hRecord));
	    for (std::vector<HSong>::const_iterator s (as.begin ()); s != as.end (); ++s)
	       songs.append ((HSong&)(*s));
	 }

	 enableEdit (OBJECT_SELECTED);
      }
      else
	 enableEdit (OWNER_SELECTED);
   }
   else
      enableEdit (NONE_SELECTED);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a song
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PRecords::songSelected () {
   TRACE9 ("PRecords::songSelected ()");
   Check3 (songs.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list
      (songs.get_selection ()->get_selected_rows ());
   TRACE9 ("PRecords::songSelected () - Size: " << list.size ());
   apMenus[DELETE]->set_sensitive (list.size ());
}

//-----------------------------------------------------------------------------
/// Callback when a song is being changed
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::songChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE4 ("PRecords::songChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - " << column);

   Gtk::TreeSelection::ListHandle_Path list
      (records.get_selection ()->get_selected_rows ());
   TRACE9 ("PRecords::songChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - Selected: " << list.size ());
   Gtk::TreePath path (*list.begin ());
   aUndo.push (Undo (Undo::CHANGED, SONG, column, songs.getEntryAt (row), path, oldValue));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Callback when changing an interpret
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::interpretChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PRecords::interpretChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - " << column);

   Gtk::TreePath path (records.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, INTERPRET, column, records.getCelebrityAt (row), path, oldValue));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Callback when changing a record
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::recordChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PRecords::recordChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - " << column);

   Gtk::TreePath path (records.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, RECORD, column, records.getObjectAt (row), path, oldValue));

   if (column == 2) {     // If the record-genre was changed, copy it for songs
      HRecord rec (records.getRecordAt (row));
      TRACE9 ("PRecords::recordChanged (const HEntity& record) - "
	      << (rec ? rec->getId () : -1UL) << '/'
	      << (rec ? rec->getName ().c_str () : "Undefined"));
      Check3 (oldValue.size () == 1);

      if (relSongs.isRelated (rec)) {
	 unsigned int genre (rec->getGenre ());

	 Glib::RefPtr<Gtk::TreeModel> model (songs.get_model ());
	 Gtk::TreeSelection::ListHandle_Path list (songs.get_selection ()
						->get_selected_rows ());
	 if (list.size ()) {
	    Gtk::TreeIter iter;
	    for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
		 i != list.end (); ++i) {
	       iter = model->get_iter (*i);
	       songs.setGenre (iter, genre);
	    }
	 }
	 else {
	    Gtk::TreeModel::Children list (model->children ()); Check3 (list.size ());
	    for (Gtk::TreeIter i (list.begin ()); i != list.end (); ++i)
	       songs.setGenre (i, genre);
	 }
      }
   }

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PRecords::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   TRACE7 ("PRecords::addMenu");

   ui += ("<menuitem action='RUndo'/>"
	  "<separator/>"
	  "<menuitem action='NInterpret'/>"
	  "<menuitem action='NRecord'/>"
	  "<menuitem action='NSong'/>"
	  "<separator/>"
	  "<menuitem action='RDelete'/>"
	  "<separator/>"
	  "<menuitem action='Import'/>"
	  "</placeholder></menu>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("RUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &PRecords::undo));
   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NInterpret", Gtk::Stock::NEW,
							_("New _interpret")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PRecords::newInterpret));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NRecord", _("_New record")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PRecords::newRecord));
   grpAction->add (apMenus[NEW3] = Gtk::Action::create ("NSong", _("New _song")),
		   Gtk::AccelKey (_("<ctl><shft>N")),
		   mem_fun (*this, &PRecords::newSong));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("RDelete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PRecords::deleteSelection));
   grpAction->add (Gtk::Action::create ("Import", _("_Import from file-info ...")),
		   Gtk::AccelKey (_("<ctl>I")),
		   mem_fun (*this, &PRecords::importFromFileInfo));

   apMenus[UNDO]->set_sensitive (false);
   recordSelected ();
}

//-----------------------------------------------------------------------------
/// Imports information from audio file (e.g. MP3-ID3 tag or OGG-commentheader)
//-----------------------------------------------------------------------------
void PRecords::importFromFileInfo () {
   XGP::FileDialog::create (_("Select file(s) to import"),
			    Gtk::FILE_CHOOSER_ACTION_OPEN,
			    XGP::FileDialog::MUST_EXIST
			    | XGP::FileDialog::MULTIPLE)
      ->sigSelected.connect (mem_fun (*this, &PRecords::parseFileInfo));
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param file: Name of file to analzye
//-----------------------------------------------------------------------------
void PRecords::parseFileInfo (const std::string& file) {
   TRACE8 ("PRecords::parseFileInfo (const std::string&) - " << file);
   Check2 (file.size ());

   std::ifstream stream (file.c_str ());
   Glib::ustring artist, record, song;
   unsigned int track (0);
   if (!stream) {
      Glib::ustring err (_("Can't open file `%1'!\n\nReason: %2"));
      err.replace (err.find ("%1"), 2, file);
      err.replace (err.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog (err).run ();
      return;
   }

   std::string extension (file.substr (file.size () - 4));
   TRACE1 ("PRecords::parseFileInfo (const std::string&) - Type: " << extension);
   if (((extension == ".mp3")
	&& parseMP3Info (stream, artist, record, song, track))
       || ((extension == ".ogg")
	   && parseOGGCommentHeader (stream, artist, record, song, track))) {
      TRACE8 ("PRecords::parseFileInfo (const std::string&) - " << artist
	      << '/' << record << '/' << song << '/' << track);
      Check1 (typeid (**pages) == typeid (PRecords));
      addEntry (artist, record, song, track);
   }
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param stream: MP3-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if ID3 info has been found
//-----------------------------------------------------------------------------
bool PRecords::parseMP3Info (std::istream& stream, Glib::ustring& artist,
			      Glib::ustring& record, Glib::ustring& song,
			      unsigned int& track) {
   stream.seekg (-0x80, std::ios::end);
   std::string value;

   std::getline (stream, value, '\xff');
   TRACE8 ("PRecords::parseMP3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Found: "
	   << value << "; Length: " << value.size ());
   if ((value.size () > 3) && (value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
      song = Glib::locale_to_utf8 (stripString (value, 3, 29));
      artist = Glib::locale_to_utf8 (stripString (value, 33, 29));
      record = Glib::locale_to_utf8 (stripString (value, 63, 29));
      track = (value[0x7d] != 0x20) ? value[0x7e] : 0;
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Reads the OGG comment header from an OGG vorbis encoded file
/// \param stream: OGG-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if comment header has been found
//-----------------------------------------------------------------------------
bool PRecords::parseOGGCommentHeader (std::istream& stream, Glib::ustring& artist,
				       Glib::ustring& record, Glib::ustring& song,
				       unsigned int& track) {
   char buffer[512];
   stream.read (buffer, 4);
   if ((*buffer != 'O') && (buffer[1] != 'g') && (buffer[2] != 'g') && (buffer[3] != 'S'))
      return false;

   stream.seekg (0x69, std::ios::cur);
   unsigned int len (0);
   stream.read ((char*)&len, 4);                // Read the vendorstring-length
   TRACE8 ("PRecords::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Length: " << len);
   stream.seekg (len, std::ios::cur);

   unsigned int cComments (0);
   stream.read ((char*)&cComments, 4);               // Read number of comments
   TRACE8 ("PRecords::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Comments: " << cComments);
   if (!cComments)
      return false;

   std::string key;
   Glib::ustring *value (NULL);
   do {
      stream.read ((char*)&len, 4);                  // Read the comment-length

      std::getline (stream, key, '=');
      len -= key.size () + 1;
      TRACE8 ("PRecords::parseOGGCommentHeader (std::stream&, 3x Glib::ustring&, unsigned&) - Key: " << key);

      if (key == "TITLE")
	 value = &song;
      else if (key == "ALBUM")
	 value = &record;
      else if (key == "ARTIST")
	 value = &artist;
      else if (key == "TRACKNUMBER") {
	 Check2 (len < sizeof (buffer));
	 stream.read (buffer, len);
	 track = strtoul (buffer, NULL, 10);
	 value = NULL;
	 len = 0;
      }
      else
	 value = NULL;

      if (value) {
	 unsigned int read (0);
	 do {
	    read = stream.readsome (buffer, (len > sizeof (buffer)) ? sizeof (buffer) - 1 : len);
	    len -= read;
	    buffer[read] = '\0';
	    value->append (buffer);
	 } while (len);
      }
      else
	 stream.seekg (len, std::ios::cur);
   } while (--cComments);  // end-do while comments
   return true;
}

//-----------------------------------------------------------------------------
/// Returns the specified substring, removed from trailing spaces
/// \param value: String to manipulate
/// \param pos: Starting pos inside the string
/// \param len: Maximal length of string
/// \returns std::string: Stripped value
//-----------------------------------------------------------------------------
std::string PRecords::stripString (const std::string& value, unsigned int pos, unsigned int len) {
   len += pos;
   while (len > pos) {
      if ((value[len] != ' ') && (value[len]))
         break;
      --len;
   }
   return (pos == len) ? " " : value.substr (pos, len - pos + 1);
}

//-----------------------------------------------------------------------------
/// Adds an interpret to the record listbox
/// \param interpret: Handle to the new interpret
/// \returns Gtk::TreeIter: Iterator to new added interpret
//-----------------------------------------------------------------------------
Gtk::TreeIter PRecords::addInterpret (const HInterpret& interpret) {
   interprets.push_back (interpret);

   Gtk::TreeModel::iterator i (records.append (interpret));
   Gtk::TreePath path (records.getModel ()->get_path (i));
   records.set_cursor (path, *records.get_column (0), true);

   aUndo.push (Undo (Undo::INSERT, INTERPRET, 0, interpret, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a record to the record listbox
/// \param parent: Iterator to the interpret of the record
/// \param record: Handle to the new record
/// \returns Gtk::TreeIter: Iterator to new added record
//-----------------------------------------------------------------------------
Gtk::TreeIter PRecords::addRecord (Gtk::TreeIter& parent, HRecord& record) {
   Gtk::TreeIter i (records.append (record, *parent));
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   records.expand_row (model->get_path (parent), false);
   Gtk::TreePath path (records.getModel ()->get_path (i));
   records.set_cursor (path, *records.get_column (0), true);

   HInterpret interpret;
   interpret = records.getInterpretAt (parent);
   relRecords.relate (interpret, record);

   aUndo.push (Undo (Undo::INSERT, RECORD, 0, record, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a song to the song listbox
/// \param song: Handle to the new song
/// \returns Gtk::TreeIter: Iterator to new added song
//-----------------------------------------------------------------------------
Gtk::TreeIter PRecords::addSong (HSong& song) {
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
   Check3 (list.size ());
   Gtk::TreeIter p (records.getModel ()->get_iter (*list.begin ())); Check3 (p);

   HRecord record (records.getRecordAt (p)); Check3 (record);
   relSongs.relate (record, song);
   Gtk::TreeIter iterSong (songs.append (song));
   Gtk::TreePath pathSong (songs.getModel ()->get_path (iterSong));
   songs.set_cursor (pathSong, *songs.get_column (0), true);

   Gtk::TreePath path (*list.begin ());
   aUndo.push (Undo (Undo::INSERT, SONG, 0, song, path, ""));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return p;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PRecords::saveData () throw (std::exception) {
   TRACE5 ("PRecords::saveData () - " << aUndo.size ());

   std::vector<HEntity> aSaved;
   std::vector<HEntity>::iterator posSaved (aSaved.end ());

   while (aUndo.size ()) {
      Undo last (aUndo.top ());
      TRACE7 ("PRecords::saveData () - What: " << last.what () << '/' << last.how ());

      posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
      if ((posSaved == aSaved.end ()) || (*posSaved != last.getEntity ())) {
	 switch (last.what ()) {
	 case SONG: {
	    HSong song (boost::dynamic_pointer_cast<Song> (last.getEntity ())); Check3 (song);
	    if (last.how () == Undo::DELETE) {
	       if (song->getId ()) {
		  Check3 (song->getId () == last.column ());
		  StorageRecord::deleteSong (song->getId ());
	       }

	       std::map<HEntity, HEntity>::iterator delRel (delRelation.find (last.getEntity ()));
	       Check3 (delRel != delRelation.end ());
	       Check3 (typeid (*delRel->second) == typeid (Record));
	       delRelation.erase (delRel);
	    }
	    else {
	       HRecord hRec (relSongs.getParent (song));
	       if (!hRec->getId ()) {
		  Check3 (std::find (aSaved.begin (), aSaved.end (), hRec) == aSaved.end ());
		  Check3 (delRelation.find (hRec) == delRelation.end ());

		  HInterpret interpret  (relRecords.getParent (hRec));
		  if (!interpret->getId ()) {
		     Check3 (std::find (aSaved.begin (), aSaved.end (), interpret) == aSaved.end ());
		     Check3 (delRelation.find (interpret) == delRelation.end ());

		     SaveCelebrity::store (interpret, "Interprets", *getWindow ());
		     aSaved.insert (lower_bound (aSaved.begin (), aSaved.end (), interpret), interpret);
		  }
		  StorageRecord::saveRecord (hRec, relRecords.getParent (hRec)->getId ());
		  aSaved.insert (lower_bound (aSaved.begin (), aSaved.end (), hRec), hRec);
		  posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
	       }
	       StorageRecord::saveSong (song, hRec->getId ());
	    }
	    break; }

	 case RECORD: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Record));
	    HRecord rec (boost::dynamic_pointer_cast<Record> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (rec->getId ()) {
		  Check3 (rec->getId () == last.column ());
		  StorageRecord::deleteRecord (rec->getId ());
	       }

	       std::map<HEntity, HEntity>::iterator delRel
		  (delRelation.find (last.getEntity ()));
	       Check3 (delRel != delRelation.end ());
	       Check3 (typeid (*delRel->second) == typeid (Interpret));
	       delRelation.erase (delRel);
	    }
	    else {
	       HInterpret interpret  (relRecords.getParent (rec));
	       if (!interpret->getId ()) {
		  Check3 (std::find (aSaved.begin (), aSaved.end (), interpret) == aSaved.end ());
		  Check3 (delRelation.find (interpret) == delRelation.end ());

		  SaveCelebrity::store (interpret, "Interprets", *getWindow ());
		  aSaved.insert (lower_bound (aSaved.begin (), aSaved.end (), interpret), interpret);
		  posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
	       }
	       StorageRecord::saveRecord (rec, interpret->getId ());
	    }
	    break; }

	 case INTERPRET: {
	    Check3 (typeid (*last.getEntity ()) == typeid (Interpret));
	    HInterpret interpret (boost::dynamic_pointer_cast<Interpret> (last.getEntity ()));
	    if (last.how () == Undo::DELETE) {
	       if (interpret->getId ()) {
		  Check3 (interpret->getId () == last.column ());
		  StorageRecord::deleteInterpret (interpret->getId ());
	       }
	    }
	    else
	       SaveCelebrity::store (interpret, "Interprets", *getWindow ());
	    break; }

	 default:
	    Check1 (0);
	 } // end-switch

	 aSaved.insert (posSaved, last.getEntity ());
      }
      aUndo.pop ();
   } // end-while
   Check3 (apMenus[UNDO]);
   apMenus[UNDO]->set_sensitive (false);

   Check3 (delRelation.empty ());
}

//-----------------------------------------------------------------------------
/// Removes the selected movies or directors from the listbox. Depending movies
/// are deleted too.
//-----------------------------------------------------------------------------
void PRecords::deleteSelection () {
   if (records.has_focus ())
      deleteSelectedRecords ();
   else if (songs.has_focus ())
      deleteSelectedSongs ();

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Removes the selected records or artists from the listbox. Depending objects
/// (records or songs) are deleted too.
//-----------------------------------------------------------------------------
void PRecords::deleteSelectedRecords () {
   TRACE9 ("PRecords::deleteSelectedRecords ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (records.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (records.get_model ()->get_iter (*i)); Check3 (iter);
      if ((*iter)->parent ())                // A record is going to be deleted
	 deleteRecord (iter);
      else {                             // An interpret is going to be deleted
	 TRACE9 ("PRecords::deleteSelectedRecords () - Deleting " <<
		 iter->children ().size () << " children");
	 HInterpret interpret (records.getInterpretAt (iter)); Check3 (interpret);
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    HRecord hRecord (records.getRecordAt (child));
	    if (hRecord->needsLoading () && hRecord->getId ())
	       loadSongs (hRecord);
	    deleteRecord (child);
	 }
	 Gtk::TreePath path (records.getModel ()->get_path (iter));
	 aUndo.push (Undo (Undo::DELETE, INTERPRET, interpret->getId (), interpret, path, ""));
	 records.getModel ()->erase (iter);
      }
   }
}

//-----------------------------------------------------------------------------
/// Deletes the passed record
/// \param record: Iterator to record to delete
//-----------------------------------------------------------------------------
void PRecords::deleteRecord (const Gtk::TreeIter& record) {
   Check2 (record->children ().empty ());

   HRecord hRec (records.getRecordAt (record));
   TRACE9 ("PRecords::deleteRecord (const Gtk::TreeIter&) - Deleting record "
	   << hRec->getName ());
   Check3 (relRecords.isRelated (hRec));
   HInterpret hInterpret (relRecords.getParent (hRec)); Check3 (hInterpret);

   // Remove related songs
   TRACE3 ("PRecords::deleteRecord (const Gtk::TreeIter&) - Remove Songs");
   if (relSongs.isRelated (hRec)) {
      const std::vector<HSong>& as (relSongs.getObjects (hRec));
      for (std::vector<HSong>::const_iterator s (as.begin ()); s != as.end (); ++s)
	 deleteSong ((HSong&)(*s), hRec);
      relSongs.unrelateAll (hRec);
   }

   Check3 (delRelation.find (hRec) == delRelation.end ());

   Gtk::TreePath path (records.getModel ()->get_path (records.getOwner (hInterpret)));
   aUndo.push (Undo (Undo::DELETE, RECORD, hRec->getId (), hRec, path, ""));
   delRelation[hRec] = hInterpret;
   relRecords.unrelate (hInterpret, hRec);

   records.getModel ()->erase (record);
}

//-----------------------------------------------------------------------------
/// Removes the passed songs from the model (but not from the relations).
/// \param song: Song to delete
/// \param record: Record of song
//-----------------------------------------------------------------------------
void PRecords::deleteSong (const HSong& song, const HRecord& record) {
   TRACE9 ("PRecords::deleteSong (const HSong& song, const HRecord& record)");
   Check1 (song); Check1 (record);
   Check3 (delRelation.find (song) == delRelation.end ());

   Gtk::TreePath path (records.getModel ()->get_path (records.getObject (record)));
   aUndo.push (Undo (Undo::DELETE, SONG, song->getId (), song, path, ""));
   delRelation[song] = record;
}

//-----------------------------------------------------------------------------
/// Removes the selected songs from the listbox.
//-----------------------------------------------------------------------------
void PRecords::deleteSelectedSongs () {
   TRACE9 ("PRecords::deleteSelectedSongs ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (songs.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (songs.get_model ()->get_iter (*i)); Check3 (iter);
      HSong song (songs.getSongAt (iter)); Check3 (song);
      Check3 (relSongs.isRelated (song));
      HRecord record (relSongs.getParent (song));
      Check3 (record);
      deleteSong (song, record);

      relSongs.unrelate (record, song);
      songs.getModel ()->erase (iter);
   }
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
//-----------------------------------------------------------------------------
void PRecords::export2HTML (unsigned int fd, const std::string&) {
   std::sort (interprets.begin (), interprets.end (), &Interpret::compByName);

   // Write record-information
   for (std::vector<HInterpret>::const_iterator i (interprets.begin ());
	i != interprets.end (); ++i)
      if (relRecords.isRelated (*i)) {
	 std::stringstream output;
	 output << 'I' << **i;

	 const std::vector<HRecord>& records (relRecords.getObjects (*i));
	 Check3 (records.size ());
	 for (std::vector<HRecord>::const_iterator r (records.begin ());
	      r != records.end (); ++r)
	    output << "R" << **r;

	 TRACE9 ("PRecorsd::export2HTML (unsigned int) - Writing: " << output.str ());
	 if (::write (fd, output.str ().data (), output.str ().size ()) != (ssize_t)output.str ().size ()) {
	    Glib::ustring msg (_("Couldn't write data!\n\nReason: %1"));
	    msg.replace (msg.find ("%1"), 2, strerror (errno));
	    Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	    dlg.set_title (_("Error exporting records to HTML!"));
	    dlg.run ();
	    break;
	 }
      }
}

//-----------------------------------------------------------------------------
/// Adds a song to the list (creating record/interpret/song, if necessary)
/// \param artist: Name of artist
/// \param record: Name of record
/// \param song: Name of song
/// \param track: Number of track
//-----------------------------------------------------------------------------
void PRecords::addEntry (const Glib::ustring&artist, const Glib::ustring& record,
			 const Glib::ustring& song, unsigned int track) {
   HInterpret interpret;
   Gtk::TreeIter i (records.getOwner (artist));
   if (i == records.getModel ()->children ().end ()) {
      TRACE9 ("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding band " << artist);

      interpret.reset (new Interpret);
      interpret->setName (artist);
      i = addInterpret (interpret);
   }
   else
      interpret = records.getInterpretAt (i);

   HRecord rec;
   Gtk::TreeIter r (records.getObject (i, record));
   if (r == i->children ().end ()) {
      TRACE9 ("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding record " << record);
      rec.reset (new Record);
      rec->setSongsLoaded ();
      rec->setName (record);
      addRecord (i, rec);
   }
   else {
      rec = records.getRecordAt (r);
      records.selectRow (r);
   }

   HSong hSong;
   Gtk::TreeIter s (songs.getSong (song));
   if (s == songs.getModel ()->children ().end ()) {
      TRACE9 ("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding song " << hSong);
      hSong.reset (new Song);
      hSong->setName (song);
      if (track)
	 hSong->setTrack (track);
      addSong (hSong);
   }
   else {
      hSong = songs.getSongAt (s);
      songs.scroll_to_row (songs.getModel ()->get_path (s), 0.80);
      Glib::RefPtr<Gtk::TreeSelection> songSel (songs.get_selection ());
      songSel->select (s);
      if (track) {
	 hSong->setTrack (track);
	 Gtk::TreeRow row (*s);
	 songs.updateTrack (row, hSong->getTrack ());
      }
   }
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PRecords::undo () {
   TRACE1 ("PRecords::undo ()");
   Check3 (aUndo.size ());

   Undo last (aUndo.top ());
   switch (last.what ()) {
   case SONG:
      undoSong (last);
      break;

   case RECORD:
      undoRecord (last);
      break;

   case INTERPRET:
      undoInterpret (last);
      break;

   default:
      Check2 (0);
   } // end-switch

   aUndo.pop ();
   if (aUndo.empty ()) {
      enableSave (false);
      apMenus[UNDO]->set_sensitive (false);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a record
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoRecord (const Undo& last) {
   TRACE6 ("PRecords::undoRecord (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (records.getModel ()->get_iter (path)); Check3 (iter->parent ());

   Check3 (typeid (*last.getEntity ()) == typeid (Record));
   HRecord record (boost::dynamic_pointer_cast<Record> (last.getEntity ()));
   TRACE9 ("PRecords::undoRecord (const Undo&) - " << last.how () << ": " << record->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      switch (last.column ()) {
      case 0:
	 record->setName (last.getValue ());
	 break;

      case 1:
	 record->setYear (last.getValue ());
	 break;

      case 2:
	 record->setGenre ((unsigned int)last.getValue ()[0]);
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (iter->parent ());
      Check3 (relRecords.isRelated (record));
      relRecords.unrelate (records.getInterpretAt (iter->parent ()), record);
      records.getModel ()->erase (iter);
      iter = records.getModel ()->children ().end ();
      break;

   case Undo::DELETE: {
      std::map<HEntity, HEntity>::iterator delRel
	 (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Interpret));
      HInterpret interpret (boost::dynamic_pointer_cast<Interpret> (delRel->second));
      Gtk::TreeRow rowInterpret (*records.getOwner (interpret));

      iter = records.append (record, rowInterpret);
      path = records.getModel ()->get_path (iter);

      relRecords.relate (interpret, record);
      delRelation.erase (delRel);
      break; }

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      records.update (row);
   }
   records.set_cursor (path);
   records.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to an interpret
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoInterpret (const Undo& last) {
   TRACE6 ("PRecords::undoInterpret (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (records.getModel ()->get_iter (path));

   HInterpret interpret (boost::dynamic_pointer_cast<Interpret> (last.getEntity ())); Check3 (interpret);
   TRACE9 ("PRecords::undoInterpret (const Undo&) - " << last.how () << ": " << interpret->getName ());

   switch (last.how ())
   case Undo::CHANGED: {
      Check3 (iter); Check3 (!iter->parent ());

      switch (last.column ()) {
      case 0:
	 interpret->setName (last.getValue ());
	 break;

      case 1:
	 interpret->setLifespan (last.getValue ());
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (iter); Check3 (!iter->parent ());
      Check3 (!relRecords.isRelated (interpret));
      records.getModel ()->erase (iter);
      iter = records.getModel ()->children ().end ();
      break;

   case Undo::DELETE:
      if (iter)
	 Check3 (!iter->parent ());
      else
	 iter = records.getModel ()->children ().end ();
      iter = records.insert (interpret, iter);
      path = records.getModel ()->get_path (iter);
      break;

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      records.update (row);
   }
   records.set_cursor (path);
   records.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a song
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoSong (const Undo& last) {
   TRACE6 ("PRecords::undoSong (const Undo&) - " << last.column ());

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (records.getModel ()->get_iter (path));
   Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ());
   sel->unselect_all ();
   sel->select (iter);

   Check3 (typeid (*last.getEntity ()) == typeid (Song));
   HSong song (boost::dynamic_pointer_cast<Song> (last.getEntity ()));
   TRACE9 ("PRecords::undoSong (const Undo&) - " << last.how () << ": " << song->getName ());
   iter = songs.getSong (song); Check3 (iter);

   switch (last.how ()) {
   case Undo::CHANGED: {
      switch (last.column ()) {
      case 0:
	 song->setTrack (last.getValue ());
	 break;

      case 1:
	 song->setName (last.getValue ());
	 break;

      case 2:
	 song->setDuration (last.getValue ());
	 break;

      case 3:
	 Check3 (last.getValue ().size () == 1);
	 song->setGenre ((unsigned int)last.getValue ()[0]);
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break; }

   case Undo::INSERT:
      relSongs.unrelate (relSongs.getParent (song), song);
      songs.getModel ()->erase (iter);
      iter = songs.getModel ()->children ().end ();
      break;

   case Undo::DELETE: {
      iter = songs.insert (song, iter);

      std::map<HEntity, HEntity>::iterator delRel (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Record));
      relSongs.relate (boost::dynamic_pointer_cast<Record> (delRel->second), song);

      delRelation.erase (delRel);
      break; }

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      songs.update (row);
      path = songs.getModel ()->get_path (iter);
   }
   songs.set_cursor (path);
   songs.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Sets the focus to the record-list
//-----------------------------------------------------------------------------
void PRecords::getFocus () {
   records.grab_focus ();
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PRecords::clear () {
   TRACE9 ("PRecords::clear ()");
   relSongs.unrelateAll ();
   relRecords.unrelateAll ();
   interprets.clear ();

   songs.getModel ()->clear ();
   records.getModel ()->clear ();
   NBPage::clear ();
}
