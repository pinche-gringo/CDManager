#ifndef RECORD_H
#define RECORD_H

//$Id: Record.h,v 1.7 2004/11/14 21:23:56 markus Exp $

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

#include <YGP/AYear.h>
#include <YGP/Handle.h>
#include <YGP/Entity.h>


/**Class to hold a record
 */
class Record : public YGP::Entity {
   friend class CDManager;
   friend class RecordList;

 public:
   Record () : id (0), songsLoaded (false) { }
   Record (const Record& other) : id (other.id), name (other.name),
      year (other.year), genre (other.genre), songsLoaded (other.songsLoaded) { }
   virtual ~Record () { }

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::AYear        year;
   unsigned int      genre;
   bool              songsLoaded;

   //Prohibited manager functions
   const Record& operator= (const Record& other);
};
defineHndl(Record);

#endif
