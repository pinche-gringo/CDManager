#ifndef STORAGE_H
#define STORAGE_H

//$Id: Storage.h,v 1.1 2006/01/22 18:34:32 markus Exp $

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
#include <string>

#include <YGP/StatusObj.h>

#include "Actor.h"


/**Class to access the stored entities
 */
class Storage {
 public:
   static void startTransaction ();
   static void abortTransaction ();
   static void commitTransaction ();
   static void loadCelebrities (std::vector<HCelebrity>& target, const std::string& table, YGP::StatusObject& stat);

 private:
   Storage ();
   Storage (const Storage& other);
   virtual ~Storage ();

   const Storage& operator= (const Storage& other);
};

#endif
