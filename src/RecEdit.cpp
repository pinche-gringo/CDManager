//$Id: RecEdit.cpp,v 1.5 2004/10/28 19:11:52 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : RecordEdit
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.5 $
//AUTHOR      : Markus Schwab
//CREATED     : 17.10.2004
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

#include <vector>
#include <sstream>

#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/combobox.h>
#include <gtkmm/treeview.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/messagedialog.h>


#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Relation.h>

#include "DB.h"
#include "Interpret.h"

#include "RecEdit.h"


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
RecordEdit::RecordEdit (HRecord record)
   : XGP::XDialog (OKCANCEL), pClient (manage (new Gtk::Table (5, 3, false))),
     txtRecord (NULL),
     optArtist (manage (new Gtk::ComboBox ())),
     optGenre (manage (new Gtk::ComboBox ())),
     lstSongs (manage (new Gtk::TreeView ())),
     hRecord (record),
     mArtists (Gtk::ListStore::create (colArtists)),
     mGenres (Gtk::ListStore::create (colGenres)),
     mSongs (Gtk::ListStore::create (colSongs)) {
   set_title (_("Edit Record"));

   if (!hRecord.isDefined ())
      hRecord.define ();

   txtRecord = manage (new XGP::XAttributeEntry<Glib::ustring> (hRecord->name));

   Gtk::Label* lblRecord (manage (new Gtk::Label (_("_Record name:"), true)));
   Gtk::Label* lblArtist (manage (new Gtk::Label (_("_Artist:"), true)));
   Gtk::Label* lblYear (manage (new Gtk::Label (_("_Year:"), true)));
   Gtk::Label* lblGenre (manage (new Gtk::Label (_("_Genre:"), true)));
   Gtk::Adjustment* spinYear_adj (manage (new Gtk::Adjustment (2000.0, 1900.0, 3000.0, 1.0, 10.0, 4.0)));
   spinYear = manage (new XGP::XAttributeEntry<unsigned int, Gtk::SpinButton> (hRecord->year));

   spinYear->set_adjustment (*spinYear_adj);
   spinYear->set_increments (1.0, 10.0);
			 
   lblRecord->set_justify (Gtk::JUSTIFY_LEFT);
   lblArtist->set_justify (Gtk::JUSTIFY_LEFT);
   lblYear->set_justify (Gtk::JUSTIFY_LEFT);
   lblGenre->set_justify (Gtk::JUSTIFY_LEFT);
   spinYear->set_flags (Gtk::CAN_FOCUS);
   spinYear->set_update_policy (Gtk::UPDATE_ALWAYS);
   spinYear->set_numeric (true);
   spinYear->set_digits (0);
   spinYear->set_wrap (true);
   spinYear->set_max_length (4);
   txtRecord->set_flags (Gtk::CAN_FOCUS);
   txtRecord->grab_focus ();

   optArtist->set_flags (Gtk::CAN_FOCUS);
   optGenre->set_flags (Gtk::CAN_FOCUS);
   optArtist->set_model (mArtists); Check3 (mArtists);
   optGenre->set_model (mGenres); Check3 (mGenres);
   lstSongs->set_model (mSongs); Check3 (mSongs);

   optGenre->pack_start (colGenres.colName);
   optArtist->pack_start (colArtists.colName);
   lstSongs->append_column (_("Song"), colSongs.colName);
   lstSongs->append_column (_("Duration"), colSongs.colDuration);
   lstSongs->append_column (_("Genre"), colSongs.colGenre);

   lblRecord->set_mnemonic_widget (*txtRecord);
   lblArtist->set_mnemonic_widget (*optArtist);
   lblYear->set_mnemonic_widget (*spinYear);
   lblGenre->set_mnemonic_widget (*optGenre);

   pClient->attach (*lblRecord, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblArtist, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblYear, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*spinYear, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5); 
   pClient->attach (*txtRecord, 1, 3, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*optArtist, 1, 3, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);
   pClient->attach (*lblGenre, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*optGenre, 1, 3, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);
   pClient->attach (*lstSongs, 0, 3, 4, 5, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);

   fillGenres ();
   fillInterprets ();

   if (hRecord.isDefined ()) {
      txtRecord->set_text (hRecord->name);
      spinYear->set_value (hRecord->year);

      Check3 (YGP::RelationManager::getRelation ("songs"));
      Check3 (typeid (*YGP::RelationManager::getRelation ("songs"))
	      == typeid (YGP::Relation1_N<HRecord, HSong>));

      YGP::Relation1_N<HRecord, HSong>* relSongs
	 (dynamic_cast<YGP::Relation1_N<HRecord, HSong>*>
	  (YGP::RelationManager::getRelation ("songs")));
      Check3 (relSongs);
      Check3 (relSongs->isRelated (hRecord));
      Check3 (relSongs->getObjects (hRecord).size ());

      for (std::vector<HSong>::iterator i (relSongs->getObjects (hRecord).begin ());
	   i != relSongs->getObjects (hRecord).end (); ++i) {
	 Gtk::TreeModel::Row newSong (*mSongs->append ());
	 newSong[colSongs.entry] = *i;
	 newSong[colSongs.colName] = (*i)->name;
	 newSong[colSongs.colDuration] = (*i)->duration.toString ();
	 newSong[colSongs.colGenre] = genres[(*i)->genre];
      }
   }

   get_vbox ()->pack_start (*pClient, false, false, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RecordEdit::~RecordEdit () {
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void RecordEdit::okEvent () {
   if (!hRecord.isDefined ()) {
      hRecord.define ();
      hRecord->id = 0;
   }

   std::stringstream update;
   if (txtRecord->hasChanged ()) {
      txtRecord->commit ();
      update << "name=\"" << Glib::locale_from_utf8 (hRecord->name) << "\" ";
   }

   if (hRecord->year != (unsigned int)spinYear->get_value_as_int ()) {
      hRecord->year = spinYear->get_value_as_int ();
      if (update.str ().size ())
	 update << ", ";
      update << "year=" << hRecord->year << ' ';
   }

   Check3 (optGenre->get_active ());
   Check3 (*optGenre->get_active ());
   Glib::ustring genre ((*optGenre->get_active ())[colGenres.colName]);
   if (hRecord->genre != genre) {
      if (update.str ().size ())
	 update << ", ";
      update << "genre=\"" << (*optGenre->get_active ())[colGenres.colID] << "\" ";
   }

   YGP::Relation1_N<HInterpret, HRecord>* rel;
   Check3 (YGP::RelationManager::getRelation ("records"));
   Check3 (typeid (*YGP::RelationManager::getRelation ("records"))
	   == typeid (YGP::Relation1_N<HInterpret, HRecord>));

   rel = (dynamic_cast<YGP::Relation1_N<HInterpret, HRecord>*>
	  (YGP::RelationManager::getRelation ("records")));
   Check3 (rel);

   HInterpret hArtist (rel->getParent (hRecord));
   Check3 (hArtist.isDefined ());
   TRACE5 ("RecordEdit::okEvent () - Artist: "
	   << (hArtist.isDefined () ? hArtist->name.c_str () : "None"));

   Check3 (optArtist->get_active ());
   Check3 (*optArtist->get_active ());
   unsigned long int idArtist ((*optArtist->get_active ())[colArtists.colID]);
   if (hArtist->id != idArtist) {
      hArtist->name = (*optArtist->get_active ())[colArtists.colName];
      hArtist->id = idArtist;
      if (update.str ().size ())
	 update << ", ";
      update << "interpret=\"" << hArtist->id << "\" ";
   }

   if (update.str ().size ()) {
      try {
	 std::stringstream query;
	 if (hRecord->id)
	    query << "UPDATE Records SET " << update.str () << "WHERE id="
		  << hRecord->id;
	 else
	    query << "INSERT into Records SET " << update;
	 Database::store (query.str ().c_str ());

      }
      catch (std::exception& err) {
	 Glib::ustring msg (_("Can't actualize database!\n\nReason: %1"));
	 msg.replace (msg.find ("%1"), 2, err.what ());
	 Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
   }
}


//-----------------------------------------------------------------------------
/// Fills the genre listbox
//-----------------------------------------------------------------------------
void RecordEdit::fillGenres () {
   try {
      Database::store ("SELECT id, genre FROM Genres");

      while (Database::hasData ()) {
	 // Fill and store artist entry from DB-values
	 Gtk::TreeModel::Row row = *(mGenres->append ());
	 unsigned int id (Database::getResultColumnAsUInt (0));
	 row[colGenres.colID] = id;
	 Glib::ustring genre
	    (Glib::locale_to_utf8 (Database::getResultColumnAsString (1)));
	 row[colGenres.colName] = genre;

	 if (hRecord.isDefined () && (hRecord->genre == genre))
	    optGenre->set_active (row);

	 genres[id] = genre;
	 Database::getNextResultRow ();
      } // end-while
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available genres!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Fills the artist combobox
//-----------------------------------------------------------------------------
void RecordEdit::fillInterprets () {
   YGP::Relation1_N<HInterpret, HRecord>* rel;
   HInterpret hArtist;
   if (hRecord.isDefined ()) {
      Check3 (YGP::RelationManager::getRelation ("records"));
      Check3 (typeid (*YGP::RelationManager::getRelation ("records"))
	      == typeid (YGP::Relation1_N<HInterpret, HRecord>));

      rel = (dynamic_cast<YGP::Relation1_N<HInterpret, HRecord>*>
	     (YGP::RelationManager::getRelation ("records")));
      Check3 (rel);

      hArtist = rel->getParent (hRecord);
      Check3 (hArtist.isDefined ());
      TRACE5 ("RecordEdit::fillInterprets () - Artist: "
	      << (hArtist.isDefined () ? hArtist->name.c_str () : "None"));
   }

   try {
      Database::store ("SELECT id, name FROM Interprets");

      while (Database::hasData ()) {
	 // Fill and store artist entry from DB-values
	 Gtk::TreeModel::Row row = *(mArtists->append ());
	 unsigned int id (Database::getResultColumnAsUInt (0));
	 row[colArtists.colID] = id;
	 row[colArtists.colName] =
	    Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	 TRACE5 ("RecordEdit::fillInterprets () - Adding Artist "
		 << id << '/' << Database::getResultColumnAsString (1));

	 if (hArtist.isDefined () && (hArtist->id == id))
	    optArtist->set_active (row);

	 Database::getNextResultRow ();
      } // end-while
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available interprets!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}
