//$Id$

//PROJECT     : CDManager
//SUBSYSTEM   : Statistics
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 04.04.2010
//COPYRIGHT   : Copyright (C) 2010

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
#include <gtkmm/separator.h>
#include <gtkmm/messagedialog.h>

#include <YGP/ANumeric.h>

#include "Storage.h"

#include "Statistics.h"


Statistics* Statistics::instance (NULL);


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
Statistics::Statistics ()
   : XGP::XDialog (CANCEL), pClient (new Gtk::Table (6, 4)) {
   set_title (_("Statistical information"));

   Gtk::Label* lbl = manage (new Gtk::Label (_("The database contains:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 0, 7, 0, 1, Gtk::SHRINK, Gtk::FILL, 2, 10);

   int stats[7];
   try {
      memset (stats, '\0', sizeof (stats));
      Storage::getStatistics (stats);
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the statistical information!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog (msg, Gtk::MESSAGE_ERROR).run ();
   }
   // Add record information
#ifdef WITH_RECORDS
   lbl = manage (new Gtk::Label (_("Interprets:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[2]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Records:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 2, 3, 1, 2, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[3]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 3, 4, 1, 2, Gtk::FILL, Gtk::FILL, 5);
#endif

#ifdef WITH_MOVIES
   // Add movie information
   lbl = manage (new Gtk::Label (_("Directors:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[4]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 1, 2, 2, 3, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Movies:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 2, 3, 2, 3, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[5]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 3, 4, 2, 3, Gtk::FILL, Gtk::FILL, 5);
#endif

   // Add names and articles
#ifdef WITH_ACTORS
#  if defined WITH_RECORDS or defined WITH_MOVIES
   pClient->attach (*manage (new Gtk::HSeparator ()), 0, 7, 3, 4, Gtk::FILL, Gtk::FILL, 5, 10);
#  endif

   lbl = manage (new Gtk::Label (_("First names:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 0, 1, 4, 5, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[0]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 1, 2, 4, 5, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Articles:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 2, 3, 4, 5, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[1]).toString (), Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP));
   pClient->attach (*lbl, 3, 4, 4, 5, Gtk::FILL, Gtk::FILL, 5);
#endif

   pClient->show ();

   get_vbox ()->pack_start (*pClient, Gtk::PACK_SHRINK, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Statistics::~Statistics () {
   delete pClient;
   instance = NULL;
}


//-----------------------------------------------------------------------------
/// Creates or selects (if already existing) a dialog to change the
/// preferences.
/// \param parent: Parent window
/// \returns Settings*: Pointer to the created window
//-----------------------------------------------------------------------------
Statistics* Statistics::create (const Glib::RefPtr<Gdk::Window>& parent) {
   if (instance == NULL) {
      instance = new Statistics ();
      instance->get_window ()->set_transient_for (parent);
      instance->signal_response ().connect (mem_fun (*instance, &Statistics::free));
   }
   else
      instance->present ();
   return instance;
}
