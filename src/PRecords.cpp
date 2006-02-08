//$Id: PRecords.cpp,v 1.10 2006/02/08 02:16:26 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Records
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.10 $
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
   TRACE9 ("PRecords::songChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - " << column);

   Gtk::TreePath path (songs.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, SONG, column, path, oldValue));

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
   aUndo.push (Undo (Undo::CHANGED, INTERPRET, column, path, oldValue));

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
   TRACE9 ("PRecords::songChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&) - " << column);

   Gtk::TreePath path (records.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, RECORD, column, path, oldValue));

   if (column == 2) {     // If the record-genre was changed, copy it for songs
      HRecord rec (records.getRecordAt (row));
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

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PRecords::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   ui += ("<menuitem action='RUndo'/>"
	  "<separator/>"
	  "<menuitem action='NInterpret'/>"
	  "<menuitem action='NRecord'/>"
	  "<menuitem action='NSong'/>"
	  "<separator/>"
	  "<menuitem action='RDelete'/>"
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

   apMenus[UNDO]->set_sensitive (false);
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
   Gtk::TreePath path (records.getModel ()->get_path (i));
   records.set_cursor (path);

   aUndo.push (Undo (Undo::INSERT, INTERPRET, 0, path, ""));
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
   records.set_cursor (path);

   HInterpret interpret;
   interpret = records.getInterpretAt (parent);
   relRecords.relate (interpret, record);

   aUndo.push (Undo (Undo::INSERT, RECORD, 0, path, ""));
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

   HRecord record (records.getRecordAt (p)); Check3 (record.isDefined ());
   relSongs.relate (record, song);
   p = songs.append (song);

   Gtk::TreePath path (songs.getModel ()->get_path (p));
   songs.set_cursor (path);

   aUndo.push (Undo (Undo::INSERT, SONG, 0, path, ""));
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
   return p;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PRecords::saveData () throw (Glib::ustring) {
   TRACE5 ("PRecords::saveData () - " << aUndo.size ());

   try {
      std::vector<YGP::HEntity> aSaved;
      std::vector<YGP::HEntity>::iterator posSaved (aSaved.end ());

      while (aUndo.size ()) {
	 Undo last (aUndo.top ());

	 YGP::HEntity entity (((last.what () == SONG)
			       ? YGP::HEntity::cast (songs.getEntryAt (songs.getModel ()->get_iter (last.getPath ())))
			       : records.getObjectAt (records.getModel ()->get_iter (last.getPath ()))));
	 posSaved = lower_bound (aSaved.begin (), aSaved.end (), entity);
	 if ((posSaved == aSaved.end ()) || (*posSaved != entity)) {
	    switch (last.what ()) {
	    case SONG:
	       if (last.how () == Undo::DELETE) {
		  Check3 (typeid (*delEntries.back ()) == typeid (Song));
		  HSong song (HSong::cast (delEntries.back ()));
		  if (!song->getId ()) {
		     Check3 (song->getId () == last.column ());
		     StorageRecord::deleteSong (song->getId ());
		  }

		  std::map<YGP::HEntity, YGP::HEntity>::iterator delRel
		     (delRelation.find (delEntries.back ()));
		  Check3 (delRel != delRelation.end ());
		  Check3 (typeid (*delRel->second) == typeid (Record));
		  delRelation.erase (delRel);
		  delEntries.erase (delEntries.end () - 1);
	       }
	       else {
		  HSong song (songs.getEntryAt (songs.getModel ()->get_iter (last.getPath ())));
		  StorageRecord::saveSong (song, relSongs.getParent (song)->getId ());
	       }
	       break;

	    case RECORD:
	       if (last.how () == Undo::DELETE) {
		  Check3 (typeid (*delEntries.back ()) == typeid (Record));
		  HRecord rec (HRecord::cast (delEntries.back ()));
		  if (rec->getId ()) {
		     Check3 (rec->getId () == last.column ());
		     StorageRecord::deleteRecord (rec->getId ());
		  }

		  std::map<YGP::HEntity, YGP::HEntity>::iterator delRel
		     (delRelation.find (delEntries.back ()));
		  Check3 (delRel != delRelation.end ());
		  Check3 (typeid (*delRel->second) == typeid (Interpret));
		  delRelation.erase (delRel);
		  delEntries.erase (delEntries.end () - 1);
	       }
	       else {
		  HRecord rec (records.getRecordAt (records.getModel ()->get_iter (last.getPath ())));
		  StorageRecord::saveRecord (rec, relRecords.getParent (rec)->getId ());
	       }
	       break;

	    case INTERPRET:
	       if (last.how () == Undo::DELETE) {
		  Check3 (typeid (*delEntries.back ()) == typeid (Record));
		  HInterpret interpret (HInterpret::cast (delEntries.back ()));
		  if (interpret->getId ()) {
		     Check3 (interpret->getId () == last.column ());
		     StorageRecord::deleteInterpret (interpret->getId ());
		  }
	       }
	       else {
		  HInterpret interpret (records.getInterpretAt (records.getModel ()->get_iter (last.getPath ())));
		  StorageRecord::saveInterpret (interpret);
	       }
	       break;

	    default:
	       Check1 (0);
	    } // end-switch

	    aSaved.insert (posSaved, entity);
	 }
	 aUndo.pop ();
      } // end-while
      Check3 (apMenus[UNDO]);
      apMenus[UNDO]->set_sensitive (false);

      Check3 (delEntries.empty ());
      Check3 (delRelation.empty ());
  }
   catch (std::exception& err) {
      Glib::ustring msg (_("Error saving data!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
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
	 HInterpret interpret (records.getInterpretAt (iter)); Check3 (interpret.isDefined ());
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    HRecord hRecord (records.getRecordAt (child));
	    if (hRecord->needsLoading () && hRecord->getId ())
	       loadSongs (hRecord);
	    deleteRecord (child);
	 }
	 Gtk::TreePath path (records.getModel ()->get_path (iter));
	 aUndo.push (Undo (Undo::DELETE, INTERPRET, interpret->getId (), path, ""));
	 delEntries.push_back (YGP::HEntity::cast (interpret));
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
   HInterpret hInterpret (relRecords.getParent (hRec)); Check3 (hInterpret.isDefined ());

   // Remove related songs
   TRACE3 ("PRecords::deleteRecord (const Gtk::TreeIter&) - Remove Songs");
   if (relSongs.isRelated (hRec)) {
      for (std::vector<HSong>::iterator s (relSongs.getObjects (hRec).begin ());
	   s != relSongs.getObjects (hRec).end (); ++s)
	 deleteSong (*s, hRec);
      relSongs.unrelateAll (hRec);
   }

   Check3 (std::find (delEntries.begin (), delEntries.end (), YGP::HEntity::cast (hRec)) == delEntries.end ());
   Check3 (delRelation.find (YGP::HEntity::cast (hRec)) == delRelation.end ());

   Gtk::TreePath path (records.getModel ()->get_path (records.getOwner (hInterpret)));
   aUndo.push (Undo (Undo::DELETE, RECORD, hRec->getId (), path, ""));
   delEntries.push_back (YGP::HEntity::cast (hRec));
   delRelation[YGP::HEntity::cast (hRec)] = YGP::HEntity::cast (hInterpret);
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
   Check1 (song.isDefined ()); Check1 (record.isDefined ());
   Check3 (std::find (delEntries.begin (), delEntries.end (), YGP::HEntity::cast (song)) == delEntries.end ());
   Check3 (delRelation.find (YGP::HEntity::cast (song)) == delRelation.end ());

   Gtk::TreePath path (records.getModel ()->get_path (records.getObject (HEntity::cast (record))));
   aUndo.push (Undo (Undo::DELETE, SONG, song->getId (), path, ""));
   delEntries.push_back (YGP::HEntity::cast (song));
   delRelation[YGP::HEntity::cast (song)] = YGP::HEntity::cast (record);
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
      Check3 (record.isDefined ());
      deleteSong (song, record);

      relSongs.unrelate (record, song);
      songs.getModel ()->erase (iter);
   }
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

	 TRACE9 ("PRecorsd::export2HTML (unsigned int) - Writing: " << output.str ());
	 ::write (fd, output.str ().data (), output.str ().size ());
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
   switch (last.how ()) {
   case Undo::CHANGED: {
      HRecord record (records.getRecordAt (iter));
      TRACE9 ("PRecords::undoRecord (const Undo&) - Change " << record->getName ());

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
      break; }

   case Undo::INSERT: {
      HRecord record (records.getRecordAt (iter));
      TRACE9 ("PRecords::undoRecord (const Undo&) - Insert");
      Check3 (!relRecords.isRelated (record));
      records.getModel ()->erase (iter);
      iter = records.getModel ()->children ().end ();
      break; }

   case Undo::DELETE: {
      Check3 (typeid (*delEntries.back ()) == typeid (Record));
      HRecord record (HRecord::cast (delEntries.back ()));
      TRACE9 ("PRecords::undoRecord (const Undo&) - Delete " << record->getName ());

      std::map<YGP::HEntity, YGP::HEntity>::iterator delRel
	 (delRelation.find (delEntries.back ()));
      Check3 (typeid (*delRel->second) == typeid (Interpret));
      HInterpret interpret (HInterpret::cast (delRel->second));
      Gtk::TreeRow rowInterpret (*records.getOwner (interpret));

      iter = records.append (record, rowInterpret);
      path = records.getModel ()->get_path (iter);

      relRecords.relate (interpret, record);

      delRelation.erase (delRel);
      delEntries.erase (delEntries.end () - 1);
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
   // records.get_selection ()->select (p);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to an interpret
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoInterpret (const Undo& last) {
   TRACE6 ("PRecords::undoInterpret (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (records.getModel ()->get_iter (path)); Check3 (!iter->parent ());
   switch (last.how ()) {
   case Undo::CHANGED: {
      HInterpret interpret (records.getInterpretAt (iter));
      TRACE9 ("PRecords::undoInterpret (const Undo&) - Change " << interpret->getName ());

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
      break; }

   case Undo::INSERT: {
      HInterpret interpret (records.getInterpretAt (iter));
      TRACE9 ("PRecords::undoInterpret (const Undo&) - Insert");
      Check3 (!relRecords.isRelated (interpret));
      records.getModel ()->erase (iter);
      iter = records.getModel ()->children ().end ();
      break; }

   case Undo::DELETE: {
      Check3 (typeid (*delEntries.back ()) == typeid (Interpret));
      HInterpret interpret (HInterpret::cast (delEntries.back ()));
      TRACE9 ("PRecords::undoInterpret (const Undo&) - Delete " << interpret->getName ());
      iter = records.insert (interpret, iter);
      path = records.getModel ()->get_path (iter);

      delEntries.erase (delEntries.end () - 1);
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
   // records.get_selection ()->select (p);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a song
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoSong (const Undo& last) {
   TRACE6 ("PRecords::undoSong (const Undo&) - " << last.column ());

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (songs.getModel ()->get_iter (path));
   switch (last.how ()) {
   case Undo::CHANGED: {
      HSong song (songs.getEntryAt (iter));
      TRACE9 ("PRecords::undoSong (const Undo&) - Change " << song->getName ());

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

   case Undo::INSERT: {
      HSong song (songs.getEntryAt (iter));
      TRACE9 ("PRecords::undoSong (const Undo&) - Insert");
      relSongs.unrelate (relSongs.getParent (song), song);
      songs.getModel ()->erase (iter);
      iter = songs.getModel ()->children ().end ();
      break; }

   case Undo::DELETE: {
      Check3 (typeid (*delEntries.back ()) == typeid (Song));
      HSong song (HSong::cast (delEntries.back ()));
      TRACE9 ("PRecords::undoSong (const Undo&) - Delete " << song->getName ());
      iter = songs.insert (song, iter);
      path = songs.getModel ()->get_path (iter);

      std::map<YGP::HEntity, YGP::HEntity>::iterator delRel
	 (delRelation.find (delEntries.back ()));
      Check3 (typeid (*delRel->second) == typeid (Record));
      relSongs.relate (HRecord::cast (delRel->second), song);

      delRelation.erase (delRel);
      delEntries.erase (delEntries.end () - 1);
      break; }

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      songs.update (row);
   }
   songs.set_cursor (path);
   songs.scroll_to_row (path, 0.8);
   // songs.get_selection ()->select (p);
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
}

#endif
