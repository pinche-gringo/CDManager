//PROJECT     : CDManager
//SUBSYSTEM   : Statistics
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 04.04.2010
//COPYRIGHT   : Copyright (C) 2010, 2011

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
   : XGP::XDialog (CANCEL), pClient (new Gtk::Table (4 + WITH_RECORDS + WITH_FILMS + WITH_ACTORS, 4)) {
   set_title (_("Statistical information"));

   Gtk::Label* lbl = manage (new Gtk::Label (_("The database contains:")));
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

   unsigned int line (1);
   // Add record information
#ifdef WITH_RECORDS
   lbl = manage (new Gtk::Label (_("Interprets:")));
   pClient->attach (*lbl, 0, 1, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[2]).toString ()));
   pClient->attach (*lbl, 1, 2, line, line + 1, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Records:")));
   pClient->attach (*lbl, 2, 3, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[3]).toString ()));
   pClient->attach (*lbl, 3, 4, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   ++line;
#endif

#ifdef WITH_FILMS
   // Add film information
   lbl = manage (new Gtk::Label (_("Directors:")));
   pClient->attach (*lbl, 0, 1, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[4]).toString ()));
   pClient->attach (*lbl, 1, 2, line, line + 1, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Films:")));
   pClient->attach (*lbl, 2, 3, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[5]).toString ()));
   pClient->attach (*lbl, 3, 4, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   ++line;
#endif

#ifdef WITH_ACTORS
   // Add film information
   lbl = manage (new Gtk::Label (_("Actors:")));
   pClient->attach (*lbl, 0, 1, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[6]).toString ()));
   pClient->attach (*lbl, 1, 2, line, line + 1, Gtk::FILL, Gtk::FILL, 15);
   ++line;
#endif

   // Add names and articles
#if defined WITH_RECORDS or defined WITH_FILMS or defined WITH_ACTORS
   pClient->attach (*manage (new Gtk::HSeparator ()), 0, 7, line, line + 1, Gtk::FILL, Gtk::FILL, 5, 10);
   ++line;
#  endif

   lbl = manage (new Gtk::Label (_("First names:")));
   pClient->attach (*lbl, 0, 1, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[0]).toString ()));
   pClient->attach (*lbl, 1, 2, line, line + 1, Gtk::FILL, Gtk::FILL, 15);
   lbl = manage (new Gtk::Label (_("Articles:")));
   pClient->attach (*lbl, 2, 3, line, line + 1, Gtk::FILL, Gtk::FILL, 5);
   lbl = manage (new Gtk::Label (YGP::ANumeric (stats[1]).toString ()));
   pClient->attach (*lbl, 3, 4, line, line + 1, Gtk::FILL, Gtk::FILL, 5);

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
