#ifndef RECORD_H
#define RECORD_H

//$Id: Record.h,v 1.4 2004/10/30 17:50:54 markus Exp $

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

#include <YGP/Handle.h>
#include <YGP/Entity.h>


/**Class to hold a record
 */
class Record : public YGP::Entity {
   friend class CDManager;
   friend class RecordEdit;

 public:
   Record () : id (0) { }
   virtual ~Record () { }

 private:
   unsigned long int id;
   Glib::ustring     name;
   unsigned int      year;
   unsigned int      genre;

   //Prohibited manager functions
   Record (const Record& other);
   const Record& operator= (const Record& other);
};
defineHndl(Record);

#endif
