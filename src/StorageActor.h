#ifndef STORAGEACTOR_H
#define STORAGEACTOR_H

//$Id: StorageActor.h,v 1.1 2006/01/22 18:34:32 markus Exp $

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


#include <map>
#include <vector>
#include <string>

#include <YGP/StatusObj.h>

#include "Actor.h"

#include "Storage.h"


/**Class to access the stored ators
 */
class StorageActor : public Storage {
 public:
   static void loadActors (std::vector<HActor>& target, YGP::StatusObject& stat) {
      loadCelebrities (target, "Actors", stat); }
   static void loadActorsInMovies (std::map<unsigned int, std::vector<unsigned int> >& aActors) throw (std::exception);
   static void saveActor (const HActor actor) throw (std::exception);
   static void deleteActor (unsigned int idActor) throw (std::exception);
   static void deleteActorMovies (unsigned int idActor) throw (std::exception);
   static void saveActorMovie (unsigned int idActor, unsigned int idMovie) throw (std::exception);

 private:
   StorageActor ();
   StorageActor (const StorageActor& other);
   virtual ~StorageActor ();

   const StorageActor& operator= (const StorageActor& other);
};

#endif
