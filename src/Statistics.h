#ifndef STATISTICS_H
#define STATISTICS_H

//$Id$

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


#include <XGP/XDialog.h>

namespace Gtk {
   class Table;
}


/**Dialog to display statistical information about the CDMedia database
 */
class Statistics : public XGP::XDialog {
 public:
   static Statistics* create (const Glib::RefPtr<Gdk::Window>& parent);

   virtual ~Statistics ();

 protected:
   Statistics ();

 private:
   //Prohibited manager functions
   Statistics (const Statistics& other);
   const Statistics& operator= (const Statistics& other);

   Gtk::Table* pClient;

   static Statistics* instance;
};

#endif
