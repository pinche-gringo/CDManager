//$Id: RecEdit.cpp,v 1.2 2004/10/18 15:09:30 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : RecordEdit
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
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

#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/combobox.h>
#include <gtkmm/treeview.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/messagedialog.h>

#include "DB.h"
#include "RecEdit.h"


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
RecordEdit::RecordEdit (HRecord record)
   : XGP::XDialog (OKCANCEL), pClient (manage (new Gtk::Table (4, 3, false))),
     txtRecord (manage (new class Gtk::Entry ())),
     optArtist (manage (new Gtk::ComboBox ())),
     optGenre (manage (new Gtk::TreeView ())), hRecord (record),
     mArtists (Gtk::ListStore::create (colArtists)),
     mGenres (Gtk::ListStore::create (colGenres)) {
   set_title (_("Edit Record"));

   Gtk::Label* lblRecord (manage (new Gtk::Label (_("_Record name:"), true)));
   Gtk::Label* lblArtist (manage (new Gtk::Label (_("_Artist:"), true)));
   Gtk::Label* lblYear (manage (new Gtk::Label (_("_Year"), true)));
   Gtk::Adjustment* spinYear_adj (manage (new Gtk::Adjustment (2000.0, 1900.0, 3000.0, 1.0, 10.0, 4.0)));
   spinYear = manage (new Gtk::SpinButton (*spinYear_adj, 1, 4));
			 
   lblRecord->set_justify (Gtk::JUSTIFY_LEFT);
   lblArtist->set_justify (Gtk::JUSTIFY_LEFT);
   lblYear->set_justify (Gtk::JUSTIFY_LEFT);
   spinYear->set_flags (Gtk::CAN_FOCUS);
   spinYear->set_update_policy (Gtk::UPDATE_ALWAYS);
   spinYear->set_numeric (true);
   spinYear->set_digits (4);
   spinYear->set_wrap (true);
   txtRecord->set_flags (Gtk::CAN_FOCUS);
   txtRecord->grab_focus ();

   optArtist->set_flags (Gtk::CAN_FOCUS);
   optGenre->set_flags (Gtk::CAN_FOCUS);
   optArtist->set_model (mArtists);
   optGenre->set_model (mGenres);

   optGenre->append_column ("_Genre", colGenres.colName);
   optGenre->get_selection ()->set_mode (Gtk::SELECTION_EXTENDED);

   lblRecord->set_mnemonic_widget (*txtRecord);
   lblArtist->set_mnemonic_widget (*optArtist);
   lblYear->set_mnemonic_widget (*spinYear);

   pClient->attach (*lblRecord, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblArtist, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblYear, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*spinYear, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5); 
   pClient->attach (*txtRecord, 1, 3, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*optArtist, 1, 3, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);
   pClient->attach (*optGenre, 0, 3, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);

   fillGenres ();
   fillInterprets ();

   if (hRecord.isDefined ()) {
      txtRecord->set_text (hRecord->name);
      spinYear->set_value (hRecord->year);
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
   if (hRecord.isDefined ())
      hRecord.define ();

   hRecord->name = txtRecord->get_text ();
   hRecord->year = spinYear->get_value_as_int ();
}


//-----------------------------------------------------------------------------
/// Fills the genre option-menu
//-----------------------------------------------------------------------------
void RecordEdit::fillGenres () {
   try {
      Database::store ("SELECT id, genre FROM Genres");

      while (Database::hasData ()) {
	 // Fill and store artist entry from DB-values
	 Gtk::TreeModel::Row row = *(mGenres->append ());
	 unsigned int id (Database::getResultColumnAsUInt (0));
	 row[colGenres.colID] = id;
	 row[colGenres.colName] =
	    Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	 if (hRecord.isDefined () && (hRecord->id & id))
	    optGenre->get_selection ()->select (row);

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
/// Fills the genre option-menu
//-----------------------------------------------------------------------------
void RecordEdit::fillInterprets () {
   try {
      Database::store ("SELECT id, name FROM Interprets");

      while (Database::hasData ()) {
	 // Fill and store artist entry from DB-values
	 Gtk::TreeModel::Row row = *(mArtists->append ());
	 unsigned int id (Database::getResultColumnAsUInt (0));
	 row[colArtists.colID] = id;
	 row[colArtists.colName] =
	    Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	 if (hRecord.isDefined () && (hRecord->id == id))
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
