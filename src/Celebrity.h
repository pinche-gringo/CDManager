#ifndef CELIBRITY_H
#define CELIBRITY_H

//$Id: Celebrity.h,v 1.8 2004/11/29 18:45:59 markus Rel $

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
 public:
   Celebrity () : id (0) { }
   Celebrity (const Celebrity& other) : id (other.id), name (other.name),
      born (other.born), died (other.died) { }
   virtual ~Celebrity () { }

   static bool compByName (const HCelebrity& a, const HCelebrity& b);
   static Glib::ustring removeIgnored (const Glib::ustring& name);

   unsigned long int getId () const {return id; }
   Glib::ustring     getName () const {return name; }
   YGP::AYear        getBorn () const { return born; }
   YGP::AYear        getDied () const { return died; }

   void undefineBorn () { born.undefine (); }
   void undefineDied () { died.undefine (); }

   void setId   (const unsigned long int value) { id = value; }
   void setName (const Glib::ustring& value) { name = value; }
   void setBorn (const YGP::AYear& value) { born = value; }
   void setBorn (const std::string& value) { born = value; }
   void setDied (const YGP::AYear& value) { died = value; }
   void setDied (const std::string& value) { died = value; }

 private:
   unsigned long int id;   // %attrib%
   Glib::ustring     name; // %attrib%
   YGP::AYear        born; // %attrib%
   YGP::AYear        died; // %attrib%

   //Prohibited manager functions
   const Celebrity& operator= (const Celebrity& other);
};

#endif
