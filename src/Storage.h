#ifndef STORAGE_H
#define STORAGE_H

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
#include <string>
#include <stdexcept>

#include <YGP/StatusObj.h>

#include "Actor.h"


/**Class to access the stored entities
 */
class Storage {
 public:
   static void login (const char* db, const char* user, const char* pwd) throw (std::exception);
   static void logout ();
   static bool connected ();

   //{ \name Handling of special words
   static void loadSpecialWords () throw (std::exception);
   static void storeWord (const char* word) throw (std::exception);
   static void storeArticle (const char* article) throw (std::exception);

   static void deleteNames () throw (std::exception);
   static void deleteArticles () throw (std::exception);
   //}

   //{ \name Transaction-handling
   static void startTransaction ();
   static void abortTransaction ();
   static void commitTransaction ();
   //}

   //{ \name Handling of celebrities
   static void insertCelebrity (const HCelebrity celeb, const char* role) throw (std::exception);
   static void updateCelebrity (const HCelebrity celeb) throw (std::exception);

   static void getCelebrities (const std::string& name, std::vector<HCelebrity>& target) throw (std::exception);
   static void loadCelebrities (std::vector<HCelebrity>& target, const std::string& table,
				YGP::StatusObject& stat) throw (std::exception);

   static void setRole (unsigned int idCeleb, const char* role) throw (std::exception);
   static bool hasRole (unsigned int idCeleb, const char* role) throw (std::exception);
   //}

   static void getStatistics (int counts[7]) throw (std::exception);

 private:
   Storage ();
   Storage (const Storage& other);
   virtual ~Storage ();

   const Storage& operator= (const Storage& other);

   static void fillCelebrities (std::vector<HCelebrity>& target, YGP::StatusObject& stat);
};

#endif
