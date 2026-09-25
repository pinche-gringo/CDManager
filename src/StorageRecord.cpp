//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 24.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009, 2010

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


#include <cdmgr-cfg.h>

#include <sstream>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include "DB.h"

#include "StorageRecord.h"


//-----------------------------------------------------------------------------
/// Loads the records from the database.
/// \param aRecords: Map (with interpret-ID as index) to store the records
/// \returns unsigned int: Number of loaded records
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
unsigned int StorageRecord::loadRecords (std::map<unsigned int, std::vector<HRecord> >& aRecords,
					 YGP::StatusObject& stat) throw (std::exception) {
   db ().execute ("SELECT id, name, interpret, year, genre FROM "
		      "Records ORDER BY interpret, year");
   TRACE8 ("StorageRecord::loadRecords () - Records: " << db ().resultSize ());

   if (db ().resultSize ()) {
      HRecord newRec;
      while (db ().hasData ()) {
	 // Fill and store record entry from DB-values
	 TRACE8 ("StorageRecords::loadRecords (...) - Adding record "
		 << db ().getResultColumnAsUInt (0) << '/'
		 << db ().getResultColumnAsString (1));
	 newRec.reset (new Record);

	 try {
	    newRec->setId (db ().getResultColumnAsUInt (0));
	    newRec->setName (db ().getResultColumnAsString (1));
	    if (db ().getResultColumnAsUInt (3))
	       newRec->setYear (db ().getResultColumnAsUInt (3));
	    newRec->setGenre (db ().getResultColumnAsUInt (4));

	    aRecords[db ().getResultColumnAsUInt (2)].push_back (newRec);
	 }
	 catch (std::exception& e) {
	    Glib::ustring msg (_("Warning loading record `%1': %2"));
	    msg.replace (msg.find ("%1"), 2, newRec->getName ());
	    msg.replace (msg.find ("%2"), 2, e.what ());
	    stat.setMessage (YGP::StatusObject::WARNING, msg);
	 }

	 db ().getNextResultRow ();
      } // end-while has records
   } // endif has records

   TRACE9 ("StorageRecord::loadRecords () - Records: " << aRecords.size ());
   return db ().resultSize ();
}

//-----------------------------------------------------------------------------
/// Loads the songs for a record from the database
/// \param idRecord: ID of record whose songs shall be loaded
/// \param songs: Vector to store loaded songs
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::loadSongs (unsigned int idRecord, std::vector<HSong>& songs) throw (std::exception) {
   TRACE9 ("StorageRecord::loadSongs (unsigned int, std::vector<HSong>&) - " << idRecord);

   std::stringstream query;
   query << "SELECT id, name, duration, genre, track FROM Songs WHERE idRecord=" << idRecord;
   db ().execute (query.str ());

   HSong song;
   while (db ().hasData ()) {
      song.reset (new Song);
      song->setId (db ().getResultColumnAsUInt (0));
      song->setName (db ().getResultColumnAsString (1));
      std::string time (db ().getResultColumnAsString (2));
      if (time != "00:00:00")
	 song->setDuration (time);
      song->setGenre (db ().getResultColumnAsUInt (3));
      unsigned int track (db ().getResultColumnAsUInt (4));
      if (track)
	 song->setTrack (track);

      songs.push_back (song);
      db ().getNextResultRow ();
   } // end-while
}

//-----------------------------------------------------------------------------
/// Saves the passed record
/// \param record: Record to save
/// \param idInterpret: ID of interpret to which to record should be saved
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveRecord (const HRecord record, unsigned int idInterpret) throw (std::exception) {
   Check3 (idInterpret);

   Database::Values values;
   values ("name", db ().quote (record->getName ()))
      ("interpret", idInterpret)
      ("genre", record->getGenre ())
      ("year", record->getYear ().isDefined () ? (unsigned int)record->getYear () : 0);

   if (record->getId ()) {
      std::stringstream where;
      where << "id=" << record->getId ();
      db ().update ("Records", values, where.str ());
   }
   else {
      db ().insert ("Records", values);
      record->setId (db ().getIDOfInsert ());
   }
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveSong (const HSong song, unsigned int idRecord) throw (std::exception) {
   Check3 (idRecord);

   Database::Values values;
   values ("name", db ().quote (song->getName ()))
      ("idRecord", idRecord)
      ("duration", db ().quote (song->getDuration ().toUnformattedString ()))
      ("genre", song->getGenre ())
      ("track", song->getTrack ().isDefined () ? song->getTrack () : YGP::ANumeric (0));

   if (song->getId ()) {
      std::stringstream where;
      where << "id=" << song->getId ();
      db ().update ("Songs", values, where.str ());
   }
   else {
      db ().insert ("Songs", values);
      song->setId (db ().getIDOfInsert ());
   }
}

//-----------------------------------------------------------------------------
/// Deletes the passed song
/// \param idSongs: Song to delete
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteSong (unsigned int idSong) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Songs WHERE id=" << idSong;
   db ().execute (query.str ());
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteRecord (unsigned int idRecord) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Records WHERE id=" << idRecord;
   db ().execute (query.str ());
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteInterpret (unsigned int idInterpret) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Interprets WHERE id=" << idInterpret;
   db ().execute (query.str ());
}
