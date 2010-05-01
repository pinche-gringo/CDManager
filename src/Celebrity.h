#ifndef CELIBRITY_H
#define CELIBRITY_H

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


#include <vector>

#include <boost/shared_ptr.hpp>

#include <glibmm/ustring.h>

#include <YGP/AYear.h>
#include <YGP/Entity.h>


class Celebrity;
typedef boost::shared_ptr<YGP::Entity> HEntity;
typedef boost::shared_ptr<Celebrity>   HCelebrity;


/**Class to hold an celibrity
 */
class Celebrity : public YGP::Entity {
 public:
   Celebrity ();
   Celebrity (const Celebrity& other);
   virtual ~Celebrity ();

   Celebrity& operator= (const Celebrity& other);

   static bool compById (const HCelebrity& a, const HCelebrity& b);
   static bool compByName (const HCelebrity& a, const HCelebrity& b);
   static Glib::ustring removeIgnored (const Glib::ustring& name);

   unsigned long int getId () const {return id; }
   const Glib::ustring& getName () const {return name; }
   const YGP::AYear&    getBorn () const { return born; }
   const YGP::AYear&    getDied () const { return died; }
   Glib::ustring getLifespan () const;

   void undefineBorn () { born.undefine (); }
   void undefineDied () { died.undefine (); }

   void setId   (const unsigned long int value) { id = value; }
   void setName (const Glib::ustring& value) { name = value; }
   void setBorn (const YGP::AYear& value) { born = value; }
   void setBorn (const std::string& value) throw (std::invalid_argument) { born = value; }
   void setDied (const YGP::AYear& value) { died = value; }
   void setDied (const std::string& value) throw (std::invalid_argument) { died = value; }
   void setLifespan (const Glib::ustring& value) throw (std::invalid_argument);

 private:
   unsigned long int id;   // %attrib%; ; 0
   Glib::ustring     name; // %attrib%; Name
   YGP::AYear        born; // %attrib%; Born
   YGP::AYear        died; // %attrib%; Died
};

#endif
