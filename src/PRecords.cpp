//$Id: PRecords.cpp,v 1.3 2006/01/28 03:37:56 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Records
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.3 $
//AUTHOR      : Markus Schwab
//CREATED     : 24.01.2006
//COPYRIGHT   : Copyright (C) 2006

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

#if WITH_RECORDS == 1

#include <cstdlib>

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

#include <XGP/MessageDlg.h>

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

   songs.get_selection ()->set_mode (Gtk::SELECTION_EXTENDED);
   songs.signalChanged.connect (mem_fun (*this, &PRecords::songChanged));
   records.signalOwnerChanged.connect (mem_fun (*this, &PRecords::interpretChanged));
   records.signalObjectChanged.connect (mem_fun (*this, &PRecords::entityChanged));
   records.signalObjectGenreChanged.connect
      (mem_fun (*this, &PRecords::recordGenreChanged));

   Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ());
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
	   << (record.isDefined () ? record->getName ().c_str () : "Undefined"));
   Check1 (record.isDefined ());

   try {
      std::vector<HSong> songs_;
      StorageRecord::loadSongs (record->getId (), songs_);
      TRACE5 ("PRecords::loadSongs (const HRecord& record) - Found songs: " << songs_.size ());

      if (songs_.size ()) {
	  relSongs.relate (record, *songs_.begin ());
	  if (songs_.size () > 1)
	     relSongs.getObjects (record) = songs_;
      }
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
   HInterpret interpret;
   interpret.define ();
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

   HRecord record;
   record.define ();
   record->setSongsLoaded ();
   addRecord (p, record);
}

//-----------------------------------------------------------------------------
/// Adds a new song to the first selected record
//-----------------------------------------------------------------------------
void PRecords::newSong () {
   HSong song;
   song.define ();
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
	 HRecord hRecord (records.getRecordAt (i)); Check3 (hRecord.isDefined ());
	 if (hRecord->needsLoading () && hRecord->getId ())
	    loadSongs (hRecord);
	 Check3 (!hRecord->needsLoading ());

	 // Add related songs to the listbox
	 if (relSongs.isRelated (hRecord))
	    for (std::vector<HSong>::iterator s (relSongs.getObjects (hRecord).begin ());
		 s != relSongs.getObjects (hRecord).end (); ++s)
	       songs.append (*s);

	 enableEdit (OBJECT_SELECTED);
      }
      else
	 enableEdit (OWNER_SELECTED);
   }
   else
      enableEdit (NONE_SELECTED);
}

//-----------------------------------------------------------------------------
/// Callback when a song is being changed
/// \param song: Handle to changed song
//-----------------------------------------------------------------------------
void PRecords::songChanged (const HSong& song) {
   TRACE9 ("PRecords::songChanged (const HSong&)");
}

//-----------------------------------------------------------------------------
/// Callback when changing an interpret
/// \param entity: Handle to changed interpret
//-----------------------------------------------------------------------------
void PRecords::interpretChanged (const HInterpret& interpret) {
}

//-----------------------------------------------------------------------------
/// Callback when changing a record
/// \param entity: Handle to changed record
//-----------------------------------------------------------------------------
void PRecords::recordChanged (const HRecord& record) {
}

//-----------------------------------------------------------------------------
/// Callback when changing a record (with a HEntity as parameter)
/// \param entity: Handle to changed record
//-----------------------------------------------------------------------------
void PRecords::entityChanged (const HEntity& record) {
   recordChanged (HRecord::cast (record));
}

//-----------------------------------------------------------------------------
/// Callback (additional to recordChanged) when the genre of a record is being
/// changed
/// \param record: Handle to changed record
//-----------------------------------------------------------------------------
void PRecords::recordGenreChanged (const HEntity& record) {
   Check1 (record.isDefined ());
   HRecord rec (HRecord::cast (record));
   TRACE9 ("PRecords::recordGenreChanged (const HEntity& record) - "
	   << (rec.isDefined () ? rec->getId () : -1UL) << '/'
	   << (rec.isDefined () ? rec->getName ().c_str () : "Undefined"));

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

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PRecords::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   ui += ("<menuitem action='Undo'/>"
	  "<separator/>"
	  "<menuitem action='NInterpret'/>"
	  "<menuitem action='NRecord'/>"
	  "<menuitem action='NSong'/>"
	  "<separator/>"
	  "<menuitem action='Delete'/>"
	  "</placeholder></menu>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("Undo", Gtk::Stock::UNDO),
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
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("Delete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PRecords::deleteSelection));

   recordSelected ();
}

//-----------------------------------------------------------------------------
/// Adds an interpret to the record listbox
/// \param interpret: Handle to the new interpret
/// \returns Gtk::TreeIter: Iterator to new added interpret
//-----------------------------------------------------------------------------
Gtk::TreeIter PRecords::addInterpret (const HInterpret& interpret) {
   interprets.push_back (interpret);

   Gtk::TreeModel::iterator i (records.append (interpret));
   records.selectRow (i);
   interpretChanged (interpret);
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
   records.selectRow (i);
   recordChanged (record);

   HInterpret interpret;
   interpret = records.getInterpretAt (parent);
   relRecords.relate (interpret, record);
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

   HRecord record (records.getRecordAt (p)); Check3 (record.isDefined ());
   relSongs.relate (record, song);
   p = songs.append (song);
   songs.scroll_to_row (songs.getModel ()->get_path (p), 0.8);
   songs.get_selection ()->select (p);
   songChanged (song);
   return p;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PRecords::saveData () throw (Glib::ustring) {
   HInterpret interpret;
   try {
      while (changedInterprets.size ()) {
	 interpret = changedInterprets.begin ()->first; Check3 (interpret.isDefined ());
	 Check3 (changedInterprets.begin ()->second);

	 StorageRecord::saveInterpret (interpret);
	 changedInterprets.erase (changedInterprets.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't write interpret `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, interpret->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   HRecord record;
   try {
      while (changedRecords.size ()) {
	 record = changedRecords.begin ()->first; Check3 (record.isDefined ());
	 Check3 (changedRecords.begin ()->second);
	 Check2 (relRecords.isRelated (record));
	 Check3 (relRecords.getParent (record).isDefined ());

	 StorageRecord::saveRecord (record, relRecords.getParent (record)->getId ());
	 changedRecords.erase (changedRecords.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't write record `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, record->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   HSong song;
   try {
      while (changedSongs.size ()) {
	 song = changedSongs.begin ()->first; Check3 (song.isDefined ());
	 Check3 (changedSongs.begin ()->second);
	 Check2 (relSongs.isRelated (song));
	 Check3 (relSongs.getParent (song).isDefined ());

	 StorageRecord::saveSong (song, relSongs.getParent (song)->getId ());
	 changedSongs.erase (changedSongs.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't write song `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, song->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedSongs.size ()) {
	 song = deletedSongs.begin ()->first; Check3 (song.isDefined ());
	 if (song->getId ())
	    StorageRecord::deleteSong (song->getId ());
	 deletedSongs.erase (deletedSongs.begin ());
      } // end-while
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete song `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, song->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedRecords.size ()) {
	 record = deletedRecords.begin ()->first; Check3 (record.isDefined ());
	 if (record->getId ())
	    StorageRecord::deleteRecord (record->getId ());
	 deletedRecords.erase (deletedRecords.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete record `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, record->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedInterprets.size ()) {
	 interpret = *deletedInterprets.begin (); Check3 (interpret.isDefined ());
	 if (interpret->getId ())
	    StorageRecord::deleteInterpret (interpret->getId ());
	 deletedInterprets.erase (deletedInterprets.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete interpret `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, interpret->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }
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
	 HInterpret interpret (records.getInterpretAt (iter)); Check3 (interpret.isDefined ());
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    HRecord hRecord (records.getRecordAt (child));
	    if (hRecord->needsLoading () && hRecord->getId ())
	       loadSongs (hRecord);
	    deleteRecord (child);
	 }
	 undoInterprets.push_back (interpret);
	 records.getModel ()->erase (iter);
	 if (interpret->getId ())
	    deletedInterprets.push_back (interpret);

	 std::map<HInterpret, HInterpret>::iterator iInterpret (changedInterprets.find (interpret));
	 if (iInterpret != changedInterprets.end ())
	    changedInterprets.erase (iInterpret);
      }
   }
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
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
   HInterpret hInterpret (relRecords.getParent (hRec)); Check3 (hInterpret.isDefined ());

   // Remove related songs
   TRACE3 ("PRecords::deleteRecord (const Gtk::TreeIter&) - Remove Songs");
   while (relSongs.isRelated (hRec)) {
      std::vector<HSong>::iterator s (relSongs.getObjects (hRec).begin ());
      undoSongs.push_back (*s);
      deletedSongs[*s] = hRec;
      relSongs.unrelate (hRec, *s);
   }
   undoRecords.push_back (hRec);
   deletedRecords[hRec] = hInterpret;
   relRecords.unrelate (hInterpret, hRec);

   std::map<HRecord, HRecord>::iterator iRec (changedRecords.find (hRec));
   if (iRec != changedRecords.end ())
      changedRecords.erase (iRec);

   // Delete interpret from listbox if it doesn't have any records
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   model->erase (record);
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
      HSong song (songs.getEntryAt (iter)); Check3 (song.isDefined ());
      Check3 (relSongs.isRelated (song));
      HRecord record (relSongs.getParent (song));

      undoSongs.push_back (song);
      deletedSongs[song] = record;
      relSongs.unrelate (record, song);

      std::map<HSong, HSong>::iterator iSong (changedSongs.find (song));
      if (iSong != changedSongs.end ())
	 changedSongs.erase (iSong);
      songs.getModel ()->erase (iter);
   }

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
//-----------------------------------------------------------------------------
void PRecords::export2HTML (unsigned int fd) {
   std::sort (interprets.begin (), interprets.end (), &Interpret::compByName);

   // Write record-information
   for (std::vector<HInterpret>::const_iterator i (interprets.begin ());
	i != interprets.end (); ++i)
      if (relRecords.isRelated (*i)) {
	 std::stringstream output;
	 output << 'I' << **i;

	 std::vector<HRecord>& records (relRecords.getObjects (*i));
	 Check3 (records.size ());
	 for (std::vector<HRecord>::const_iterator r (records.begin ());
	      r != records.end (); ++r)
	    output << "R" << **r;

	 TRACE9 ("CDManager::export () - Writing: " << output.str ());
	 ::write (fd, output.str ().data (), output.str ().size ());
      }
}

//-----------------------------------------------------------------------------
/// Adds a song to the list (creating record/interpret, if necessary)
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
      interpret.define ();
      interpret->setName (artist);
      i = addInterpret (interpret);
   }
   else
      interpret = records.getInterpretAt (i);

   HRecord rec;
   Gtk::TreeIter r (records.getObject (i, record));
   if (r == i->children ().end ()) {
      TRACE9 ("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding record " << record);
      rec.define ();
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
      hSong.define ();
      hSong->setName (song);
      if (track)
	 hSong->setTrack (track);
      addSong (hSong);
   }
   else {
      hSong = songs.getEntryAt (s);
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
   for (std::vector<HInterpret>::iterator i (interprets.begin ());
	i != interprets.end (); ++i)
      if (relRecords.isRelated (*i)) {
	 std::vector<HRecord>& records (relRecords.getObjects (*i));
	 for (std::vector<HRecord>::iterator r (records.begin ());
	      r != records.end (); ++r)
	    if (relSongs.isRelated (*r))
	       relSongs.unrelateAll (*r);
	 relRecords.unrelateAll (*i);
      }
   interprets.clear ();

   songs.getModel ()->clear ();
}

#endif
