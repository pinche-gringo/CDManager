#ifndef SETTINGS_H
#define SETTINGS_H

//$Id: Settings.h,v 1.1 2004/12/24 04:08:43 markus Exp $

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


#include <string>

#include <gtkmm/entry.h>

#include <XGP/XAttrEntry.h>

#include <XGP/XDialog.h>


// Forward declarations
class Options;

namespace Gtk {
   class Table;
}


class Settings : public XGP::XDialog {
 public:
   virtual ~Settings ();

   static Settings* create (const Glib::RefPtr<Gdk::Window>& parent,
			    Options& options) {
      Settings* dlg (new Settings (options));
      dlg->get_window ()->set_transient_for (parent);
      dlg->signal_response ().connect (mem_fun (*dlg, &Settings::free));
      return dlg;
   }

 protected:
   Settings (Options& options);

 private:
   //Prohibited manager functions
   Settings (const Settings& other);
   const Settings& operator= (const Settings& other);

   virtual void okEvent ();

   XGP::XAttributeEntry<std::string> txtOutput;
   XGP::XAttributeEntry<std::string> hdrMovie;
   XGP::XAttributeEntry<std::string> ftrMovie;
   XGP::XAttributeEntry<std::string> hdrRecord;
   XGP::XAttributeEntry<std::string> ftrRecord;

   Gtk::Table* pClient;

   static XGP::XAttributeEntry<std::string> Settings::* fields[];
};

#endif
