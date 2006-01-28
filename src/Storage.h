#ifndef STORAGE_H
#define STORAGE_H

//$Id: Storage.h,v 1.3 2006/01/28 01:14:44 markus Rel $

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
   static void login (const char* db, const char* user, const char* pwd) throw (std::exception);
   static void logout ();

   //{ \name handling of special words
   static void loadSpecialWords () throw (std::exception);
   static void storeWord (const char* word) throw (std::exception);
   static void storeArticle (const char* article) throw (std::exception);

   static void deleteNames () throw (std::exception);
   static void deleteArticles () throw (std::exception);
   //}

   static void startTransaction ();
   static void abortTransaction ();
   static void commitTransaction ();
   static void loadCelebrities (std::vector<HCelebrity>& target, const std::string& table,
				YGP::StatusObject& stat) throw (std::exception);

 private:
   Storage ();
   Storage (const Storage& other);
   virtual ~Storage ();

   const Storage& operator= (const Storage& other);
};

#endif
