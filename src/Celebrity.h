#ifndef CELIBRITY_H
#define CELIBRITY_H

//$Id: Celebrity.h,v 1.4 2004/11/25 23:20:55 markus Exp $

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


#include <vector>

#include <glibmm/ustring.h>

#include <YGP/AYear.h>
#include <YGP/Handle.h>
#include <YGP/Entity.h>


class Celebrity;
defineHndl(Celebrity);


/**Class to hold an celibrity
 */
class Celebrity : public YGP::Entity {
   friend class CDManager;
   friend class RecordList;
   friend class MovieList;

 public:
   Celebrity () : id (0) { }
   Celebrity (const Celebrity& other) : id (other.id), name (other.name),
      born (other.born), died (other.died) { }
   virtual ~Celebrity () { }

   static bool compByName (const HCelebrity& a, const HCelebrity& b);

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::AYear        born;
   YGP::AYear        died;

   static Glib::ustring removeIgnored (const Glib::ustring& name);

   //Prohibited manager functions
   const Celebrity& operator= (const Celebrity& other);

   static std::vector<Glib::ustring> ignore;
};

#endif
