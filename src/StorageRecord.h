#ifndef STORAGERECORD_H
#define STORAGERECORD_H

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

#include <YGP/Relation.h>

#include "Song.h"
#include "Record.h"
#include "Interpret.h"

#include "Storage.h"


// Forward declarations
namespace YGP {
   class StatusObject;
};


/**Class to access the stored records
 */
class StorageRecord : public Storage {
 public:
   static void loadInterprets (std::vector<HInterpret>& target, YGP::StatusObject& stat) {
      loadCelebrities (target, "Interprets", stat); }
   static unsigned int loadRecords (std::map<unsigned int, std::vector<HRecord> >& aRecords,
				    YGP::StatusObject& stat) throw (std::exception);
   static void loadSongs (unsigned int idRecord, std::vector<HSong>& songs) throw (std::exception);

   static void saveSong (const HSong song, unsigned int idRecord) throw (std::exception);
   static void saveRecord (const HRecord record, unsigned int idInterpret) throw (std::exception);
   static void deleteSong (unsigned int idSong) throw (std::exception);
   static void deleteRecord (unsigned int idRecord) throw (std::exception);
   static void deleteInterpret (unsigned int idInterpret) throw (std::exception);

   static void loadNames (const std::vector<HInterpret>& interprets,
			  const YGP::Relation1_N<HInterpret, HRecord>& relRecords,
			  const std::string& lang) throw (std::exception);

 private:
   StorageRecord ();
   StorageRecord (const StorageRecord& other);
   virtual ~StorageRecord ();

   const StorageRecord& operator= (const StorageRecord& other);
};

#endif
