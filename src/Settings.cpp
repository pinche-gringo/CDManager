//PROJECT     : CDManager
//SUBSYSTEM   : Settings
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 23.12.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011

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

#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/notebook.h>

#include "Words.h"
#include "WordDlg.h"
#include "Options.h"
#include "Options.meta"

#include "Settings.h"


XGP::XAttributeEntry<std::string> Settings::* Settings::fields[] =
   { &Settings::txtOutput, &Settings::hdrFilm, &Settings::ftrFilm,
     &Settings::hdrRecord, &Settings::ftrRecord };

Settings* Settings::instance (NULL);


//-----------------------------------------------------------------------------
/// Constructor
/// \param options: Options to change
//-----------------------------------------------------------------------------
Settings::Settings (Options& options)
   : XGP::XDialog (OKCANCEL),
     txtOutput (options.dirOutput), hdrFilm (options.mHeader),
     ftrFilm (options.mFooter), hdrRecord (options.rHeader),
     ftrRecord (options.rFooter),
     wordDialog (WordDialog::makeDialog ()) {
   Check3 (!instance);
   instance =  this;

   set_title (_("Preferences"));
   set_size_request (450, 350);

   Gtk::Notebook& nb (*manage (new Gtk::Notebook));
   Gtk::Table& pagExport (*manage (new Gtk::Table (5, 2)));

   Glib::ustring lbls[sizeof (fields) / sizeof (*fields)] =
      { _("Output _directory:"), _("_Header for films:"),
	_("_Footer for films:"), _("Header for _records:"),
	_("Foo_ter for records:") };

   Gtk::Label* lbl;
   for (unsigned int i (0); i < (sizeof (fields) / sizeof (*fields)); ++i) {
      lbl = manage (new Gtk::Label (lbls[i], true));
      lbl->set_mnemonic_widget (this->*fields[i]);
      pagExport.attach (*lbl, 0, 1, i, i + 1, Gtk::FILL, Gtk::FILL, 5);
      pagExport.attach (this->*fields[i], 1, 2, i, i + 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5);
   }

   nb.append_page (*manage (wordDialog), _("Reserved _words"), true);
   nb.append_page (pagExport, _("_Export"), true);

   get_vbox ()->pack_start (nb, true, true, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Settings::~Settings () {
   instance = NULL;
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void Settings::okEvent () {
   ok->grab_focus ();
   for (unsigned int i (0); i < (sizeof (fields) / sizeof (*fields)); ++i)
      (this->*fields[i]).commit ();

   WordDialog::commitDialogData (wordDialog);
}

//-----------------------------------------------------------------------------
/// Creates or selects (if already existing) a dialog to change the
/// preferences.
/// \param parent: Parent window
/// \returns Settings*: Pointer to the created window
//-----------------------------------------------------------------------------
Settings* Settings::create (const Glib::RefPtr<Gdk::Window>& parent,
			    Options& options) {
   if (instance == NULL) {
      new Settings (options); Check3 (instance);
      instance->get_window ()->set_transient_for (parent);
      instance->signal_response ().connect (mem_fun (*instance, &Settings::free));
   }
   else
      instance->present ();
   return instance;
}
