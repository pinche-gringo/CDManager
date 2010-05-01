#ifndef CDTYPE_H
#define CDTYPE_H

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


#include <YGP/MetaEnum.h>


/**Class to hold the types of the CDs
 */
class CDType : public YGP::MetaEnum {
 public:
   static CDType& getInstance () {
      if (!instance)
	 instance = new CDType;
      return *instance; }
   ~CDType ();

 private:
   //Prohibited manager functions
   CDType ();
   CDType (const CDType&);
   const CDType& operator= (const CDType& other);

   static CDType* instance;
};

#endif
