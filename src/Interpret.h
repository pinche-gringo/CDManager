#ifndef INTERPRET_H
#define INTERPRET_H

//$Id: Interpret.h,v 1.1 2004/10/22 03:45:23 markus Rel $

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


#include <glibmm/ustring.h>

#include <YGP/ADate.h>
#include <YGP/Handle.h>
#include <YGP/Entity.h>


/**Class to hold an interpret
 */
class Interpret : public YGP::Entity {
   friend class CDManager;
   friend class RecordEdit;

 public:
   Interpret () { }
   virtual ~Interpret () { }

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::ADate        born;
   YGP::ADate        died;

   //Prohibited manager functions
   Interpret (const Interpret& other);
   const Interpret& operator= (const Interpret& other);
};
defineHndl(Interpret);

#endif