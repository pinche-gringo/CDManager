//$Id: RecEdit.cpp,v 1.8 2004/11/01 23:59:05 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : RecordEdit
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.8 $
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
#include <gtkmm/adjustment.h>
#include <gtkmm/messagedialog.h>


#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Relation.h>

#include "DB.h"
#include "Interpret.h"

#include "SongList.h"

#include "RecEdit.h"


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
RecordEdit::RecordEdit (HRecord record, std::vector<HInterpret>& artists,
			const std::map<unsigned int, Glib::ustring> genres)
   : XGP::XDialog (OKCANCEL),
     hRecord (record),
     pClient (manage (new Gtk::Table (5, 3, false))),
     txtRecord (NULL),
     optArtist (manage (new Gtk::ComboBox ())),
     optGenre (manage (new Gtk::ComboBox ())),
     lstSongs (manage (new SongList (genres))),
     mArtists (Gtk::ListStore::create (colArtists)),
     mGenres (Gtk::ListStore::create (colGenres)),
     artists (artists),
     genres (genres) {
   set_title (_("Edit Record"));
   Check3 (genres.size ());

   if (!hRecord.isDefined ())
      hRecord.define ();

   txtRecord = manage (new XGP::XAttributeEntry<Glib::ustring> (hRecord->name));

   Gtk::Label* lblRecord (manage (new Gtk::Label (_("_Record name:"), true)));
   Gtk::Label* lblArtist (manage (new Gtk::Label (_("_Artist:"), true)));
   Gtk::Label* lblYear (manage (new Gtk::Label (_("_Year:"), true)));
   Gtk::Label* lblGenre (manage (new Gtk::Label (_("_Genre:"), true)));
   Gtk::Adjustment* spinYear_adj (manage (new Gtk::Adjustment (2000.0, 0.0, 3000.0, 1.0, 10.0, 4.0)));
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

   optGenre->pack_start (colGenres.colName);
   optArtist->pack_start (colArtists.colName);

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

   if (hRecord->year || (hRecord->id))
      spinYear->set_value (hRecord->year);

   Check3 (YGP::RelationManager::getRelation ("songs"));
   Check3 (typeid (*YGP::RelationManager::getRelation ("songs"))
	   == typeid (YGP::Relation1_N<HRecord, HSong>));

   YGP::Relation1_N<HRecord, HSong>* relSongs
      (dynamic_cast<YGP::Relation1_N<HRecord, HSong>*>
       (YGP::RelationManager::getRelation ("songs")));
   Check3 (relSongs);
   if (relSongs->isRelated (hRecord)) {
      Check3 (relSongs->getObjects (hRecord).size ());

      for (std::vector<HSong>::iterator i (relSongs->getObjects (hRecord).begin ());
	   i != relSongs->getObjects (hRecord).end (); ++i)
	 lstSongs->append (*i);
   } // end-if

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
   Check (hRecord.isDefined ());

   ok->grab_focus ();

   std::stringstream update;
   if (txtRecord->hasChanged ()) {
      txtRecord->commit ();
      update << "name=\"" << Glib::locale_from_utf8 (hRecord->name) << '"';
   }

   if (hRecord->year != (unsigned int)spinYear->get_value_as_int ()) {
      hRecord->year = spinYear->get_value_as_int ();
      if (update.str ().size ())
	 update << ", ";
      update << "year=" << hRecord->year;
   }

   Check3 (optGenre->get_active ());
   Check3 (*optGenre->get_active ());
   unsigned int genre ((*optGenre->get_active ())[colGenres.colID]);
   if (hRecord->genre != genre) {
      hRecord->genre = (*optGenre->get_active ())[colGenres.colID];
      if (update.str ().size ())
	 update << ", ";
      update << "genre=\"" << hRecord->genre << '"';
   }

   YGP::Relation1_N<HInterpret, HRecord>* rel;
   Check3 (YGP::RelationManager::getRelation ("records"));
   Check3 (typeid (*YGP::RelationManager::getRelation ("records"))
	   == typeid (YGP::Relation1_N<HInterpret, HRecord>));

   rel = (dynamic_cast<YGP::Relation1_N<HInterpret, HRecord>*>
	  (YGP::RelationManager::getRelation ("records")));
   Check3 (rel);

   HInterpret hArtist;
   if (rel->isRelated (hRecord))
      hArtist = rel->getParent (hRecord);
   else
      hArtist.define ();
   Check3 (hArtist.isDefined ());
   TRACE5 ("RecordEdit::okEvent () - Artist: " << hArtist->name);

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
	    query << "UPDATE Records SET " << update.str () << " WHERE id="
		  << hRecord->id;
	 else
	    query << "INSERT into Records SET " << update.str ();
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
   TRACE9 ("RecordEdit::fillGenres () - Genres: " << genres.size ());

   for (std::map<unsigned int, Glib::ustring>::const_iterator i (genres.begin ());
	i != genres.end (); ++i) {
      // Fill and store artist genre entry
	 Gtk::TreeModel::Row row = *(mGenres->append ());
	 row[colGenres.colID] = i->first;
	 row[colGenres.colName] = i->second;

	 if (hRecord.isDefined () && (hRecord->genre == i->first))
	    optGenre->set_active (row);
   } // end-for
}

//-----------------------------------------------------------------------------
/// Fills the artist combobox
//-----------------------------------------------------------------------------
void RecordEdit::fillInterprets () {
   // Find artist to record
   YGP::Relation1_N<HInterpret, HRecord>* rel;
   HInterpret hArtist;
   if (hRecord.isDefined ()) {
      Check3 (YGP::RelationManager::getRelation ("records"));
      Check3 (typeid (*YGP::RelationManager::getRelation ("records"))
	      == typeid (YGP::Relation1_N<HInterpret, HRecord>));

      rel = (dynamic_cast<YGP::Relation1_N<HInterpret, HRecord>*>
	     (YGP::RelationManager::getRelation ("records")));
      Check3 (rel);

      if (rel->isRelated (hRecord)) {
	 hArtist = rel->getParent (hRecord);
	 Check3 (hArtist.isDefined ());
	 optArtist->set_sensitive (false);
      } 
      TRACE5 ("RecordEdit::fillInterprets () - Artist: "
	      << (hArtist.isDefined () ? hArtist->name.c_str () : "None"));
   }

   for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	i != artists.end (); ++i) {
      Check3 (i->isDefined ());
      TRACE5 ("RecordEdit::fillInterprets () - Adding Artist "
	      << (*i)->id << '/' << (*i)->name);

      // Fill and store artist entry
      Gtk::TreeModel::Row row = *(mArtists->append ());
      row[colArtists.colID] = (*i)->id;
      row[colArtists.colName] = (*i)->name;

      if (hArtist.isDefined () && (hArtist->id == (*i)->id))
	 optArtist->set_active (row);
   } // end-for
}
