#ifndef CELIBRITY_H
#define CELIBRITY_H

//$Id

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

#include <YGP/ADate.h>
#include <YGP/Handle.h>
#include <YGP/Entity.h>


class Celibrity;
defineHndl(Celibrity);


/**Class to hold an celibrity
 */
class Celibrity : public YGP::Entity {
   friend class CDManager;
   friend class RecordList;
   friend class MovieList;

 public:
   Celibrity () : id (0) { }
   Celibrity (const Celibrity& other) : id (other.id), name (other.name),
      born (other.born), died (other.died) { }
   virtual ~Celibrity () { }

   static bool compByName (const HCelibrity& a, const HCelibrity& b);

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::ADate        born;
   YGP::ADate        died;

   static Glib::ustring removeIgnored (const Glib::ustring& name);

   //Prohibited manager functions
   const Celibrity& operator= (const Celibrity& other);

   static std::vector<Glib::ustring> ignore;
};
defineHndl(Celibrity);

#endif
