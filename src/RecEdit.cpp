//$Id: RecEdit.cpp,v 1.1 2004/10/18 05:44:41 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : RecordEdit
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
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
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/combobox.h>

#include "RecEdit.h"


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
RecordEdit::RecordEdit (HRecord record)
   : XGP::XDialog (OKCANCEL), pClient (manage (new Gtk::Table (4, 3, false))),
     txtRecord (manage (new class Gtk::Entry ())),
     optArtist (manage (new Gtk::ComboBox ())),
     optGenre (manage (new Gtk::ComboBox ())), hRecord (record) {
   set_title (_("Edit Record"));

   Gtk::Label* lblRecord (manage (new Gtk::Label (_("_Record name:"), true)));
   Gtk::Label* lblArtist (manage (new Gtk::Label (_("_Artist:"), true)));
   Gtk::Label* lblYear (manage (new Gtk::Label (_("_Year"), true)));
   Gtk::Adjustment* spinYear_adj (manage (new Gtk::Adjustment (2000.0, 1900.0, 3000.0, 1.0, 10.0, 4.0)));
   spinYear = manage (new Gtk::SpinButton (*spinYear_adj, 1, 4));
   Gtk::Label* lblGenre (manage (new Gtk::Label (_("_Genre:"), true)));
			 
   lblRecord->set_justify (Gtk::JUSTIFY_LEFT);
   lblArtist->set_justify (Gtk::JUSTIFY_LEFT);
   lblYear->set_justify (Gtk::JUSTIFY_LEFT);
   lblGenre->set_justify (Gtk::JUSTIFY_LEFT);
   spinYear->set_flags (Gtk::CAN_FOCUS);
   spinYear->set_update_policy (Gtk::UPDATE_ALWAYS);
   spinYear->set_numeric (true);
   spinYear->set_digits (4);
   spinYear->set_wrap (true);
   txtRecord->set_flags (Gtk::CAN_FOCUS);
   txtRecord->grab_focus ();
   optArtist->set_flags (Gtk::CAN_FOCUS);
   optGenre->set_flags (Gtk::CAN_FOCUS);

   lblRecord->set_mnemonic_widget (*txtRecord);
   lblArtist->set_mnemonic_widget (*optArtist);
   lblYear->set_mnemonic_widget (*spinYear);
   lblGenre->set_mnemonic_widget (*optGenre);

   pClient->attach (*lblRecord, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblArtist, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblYear, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*spinYear, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5); 
   pClient->attach (*txtRecord, 1, 3, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*lblGenre, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK, 5, 5);
   pClient->attach (*optArtist, 1, 3, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);
   pClient->attach (*optGenre, 1, 3, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND, 5, 5);

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
}

//-----------------------------------------------------------------------------
/// Fills the genre option-menu
//-----------------------------------------------------------------------------
void RecordEdit::fillInterprets () {
   
}
