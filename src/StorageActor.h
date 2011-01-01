#ifndef STORAGEACTOR_H
#define STORAGEACTOR_H


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


#include <map>
#include <vector>
#include <string>

#include "Actor.h"

#include "Storage.h"


// Forward declarations
namespace YGP {
   class StatusObject;
};


/**Class to access the stored ators
 */
class StorageActor : public Storage {
 public:
   static void loadActors (std::vector<HActor>& target, YGP::StatusObject& stat) {
      loadCelebrities (target, "Actors", stat); }
   static void loadActorsInFilms (std::map<unsigned int, std::vector<unsigned int> >& aActors) throw (std::exception);
   static void deleteActor (unsigned int idActor) throw (std::exception);
   static void deleteActorFilms (unsigned int idActor) throw (std::exception);
   static void saveActorFilm (unsigned int idActor, unsigned int idFilm) throw (std::exception);

 private:
   StorageActor ();
   StorageActor (const StorageActor& other);
   virtual ~StorageActor ();

   const StorageActor& operator= (const StorageActor& other);
};

#endif
