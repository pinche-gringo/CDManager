//$Id: Settings.cpp,v 1.1 2004/12/24 04:08:43 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Settings
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 23.12.2004
//COPYRIGHT   : Copyright (C) 2004

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


#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include "Options.h"
#include "Options.meta"

#include "Settings.h"


XGP::XAttributeEntry<std::string> Settings::* Settings::fields[] =
   { &Settings::txtOutput, &Settings::hdrMovie, &Settings::ftrMovie,
     &Settings::hdrRecord, &Settings::ftrRecord };


//-----------------------------------------------------------------------------
/// Constructor
/// \param options: Options to change
//-----------------------------------------------------------------------------
Settings::Settings (Options& options)
   : XGP::XDialog (OKCANCEL),
     txtOutput (options.dirOutput), hdrMovie (options.mHeader),
     ftrMovie (options.mFooter), hdrRecord (options.rHeader),
     ftrRecord (options.rFooter), pClient (new Gtk::Table (5, 2)) {
   set_title (_("Preferences"));

   Glib::ustring lbls[sizeof (fields) / sizeof (*fields)] =
      { _("Output _directory:"), _("_Header for movies:"),
	_("_Footer for movies:"), _("Header for _records:"),
	_("Foo_ter for records:") };

   Gtk::Label* lbl;
   for (unsigned int i (0); i < (sizeof (fields) / sizeof (*fields)); ++i) {
      lbl = new Gtk::Label (lbls[i], true);
      lbl->set_mnemonic_widget (this->*fields[i]);
      pClient->attach (*lbl, 0, 1, i, i + 1, Gtk::FILL, Gtk::FILL, 5);
      pClient->attach (this->*fields[i], 1, 2, i, i + 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5);
   }

   pClient->show ();

   get_vbox ()->pack_start (*pClient, false, false, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Settings::~Settings () {
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void Settings::okEvent () {
   ok->grab_focus ();
   for (unsigned int i (0); i < (sizeof (fields) / sizeof (*fields)); ++i)
      (this->*fields[i]).commit ();
}
