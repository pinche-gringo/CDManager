#ifndef CDTYPE_H
#define CDTYPE_H

//$Id: CDType.h,v 1.1 2005/01/20 04:54:36 markus Rel $

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
