//PROJECT     : CDManager
//SUBSYSTEM   : Statistics
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 04.04.2010
//COPYRIGHT   : Copyright (C) 2010, 2011, 2026

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

#include <cstring>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/messagedialog.h>

#include <YGP/ANumeric.h>

#include "Storage.h"

#include "Statistics.h"


Statistics* Statistics::instance (nullptr);


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
Statistics::Statistics ()
   : XGP::XDialog (CANCEL), pClient (Gtk::make_managed<Gtk::Grid> ()) {
   set_title (_("Statistical information"));

   pClient->set_column_spacing (10);
   pClient->set_row_spacing (5);
   pClient->set_margin (5);

   Gtk::Label* lbl (Gtk::make_managed<Gtk::Label> (_("The database contains:")));
   lbl->set_margin_bottom (5);
   pClient->attach (*lbl, 0, 0, 4, 1);

   int stats[7];
   try {
      memset (stats, '\0', sizeof (stats));
      Storage::getStatistics (stats);
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the statistical information!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, false, Gtk::MessageType::ERROR);
      XGP::runModal (dlg);
   }

   unsigned int line (1);
   // Add record information
#ifdef WITH_RECORDS
   addLine (line++, _("Interprets:"), stats[2], _("Records:"), stats[3]);
#endif

#ifdef WITH_FILMS
   // Add film information
   addLine (line++, _("Directors:"), stats[4], _("Films:"), stats[5]);
#endif

#ifdef WITH_ACTORS
   // Add film information
   addLine (line++, _("Actors:"), stats[6]);
#endif

   // Add names and articles
#if defined WITH_RECORDS or defined WITH_FILMS or defined WITH_ACTORS
   Gtk::Separator* sep (Gtk::make_managed<Gtk::Separator> (Gtk::Orientation::HORIZONTAL));
   sep->set_margin_top (5);
   sep->set_margin_bottom (5);
   pClient->attach (*sep, 0, line++, 4, 1);
#  endif

   addLine (line, _("First names:"), stats[0], _("Articles:"), stats[1]);

   get_content_area ()->append (*pClient);
   show ();
}

//-----------------------------------------------------------------------------
/// Adds a line with (one or two) titles and their values
/// \param line Line (in the grid) to add
/// \param title1 First title
/// \param value1 Value to the first title
/// \param title2 Second title; if empty, only the first title is added
/// \param value2 Value to the second title
//-----------------------------------------------------------------------------
void Statistics::addLine (unsigned int line, const Glib::ustring& title1, int value1,
			  const Glib::ustring& title2, int value2) {
   pClient->attach (*Gtk::make_managed<Gtk::Label> (title1), 0, line);
   pClient->attach (*Gtk::make_managed<Gtk::Label> (YGP::ANumeric (value1).toString ()), 1, line);
   if (title2.size ()) {
      pClient->attach (*Gtk::make_managed<Gtk::Label> (title2), 2, line);
      pClient->attach (*Gtk::make_managed<Gtk::Label> (YGP::ANumeric (value2).toString ()), 3, line);
   }
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Statistics::~Statistics () {
   instance = nullptr;
}


//-----------------------------------------------------------------------------
/// Creates or selects (if already existing) a dialog to change the
/// preferences.
/// \param parent: Parent window
/// \returns Settings*: Pointer to the created window
//-----------------------------------------------------------------------------
Statistics* Statistics::create (Gtk::Window& parent) {
   if (instance == nullptr) {
      instance = new Statistics ();
      instance->set_transient_for (parent);
      instance->signal_response ().connect (sigc::mem_fun (*instance, &Statistics::free));
   }
   else
      instance->present ();
   return instance;
}
