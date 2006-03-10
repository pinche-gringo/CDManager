//$Id: StorageRecord.cpp,v 1.7 2006/03/10 21:05:40 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : <FILLIN>
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.7 $
//AUTHOR      : Markus Schwab
//CREATED     : 24.01.2006
//COPYRIGHT   : Copyright (C) 2006

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
   Database::execute ("SELECT id, name, interpret, year, genre FROM "
		      "Records ORDER BY interpret, year");
   TRACE8 ("StorageRecord::loadRecords () - Records: " << Database::resultSize ());

   if (Database::resultSize ()) {
      HRecord newRec;
      while (Database::hasData ()) {
	 // Fill and store record entry from DB-values
	 TRACE8 ("StorageRecords::loadRecords (...) - Adding record "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));
	 newRec.define ();

	 try {
	    newRec->setId (Database::getResultColumnAsUInt (0));
	    newRec->setName (Database::getResultColumnAsString (1));
	    if (Database::getResultColumnAsUInt (3))
	       newRec->setYear (Database::getResultColumnAsUInt (3));
	    newRec->setGenre (Database::getResultColumnAsUInt (4));

	    aRecords[Database::getResultColumnAsUInt (2)].push_back (newRec);
	 }
	 catch (std::exception& e) {
	    Glib::ustring msg (_("Warning loading record `%1': %2"));
	    msg.replace (msg.find ("%1"), 2, newRec->getName ());
	    msg.replace (msg.find ("%2"), 2, e.what ());
	    stat.setMessage (YGP::StatusObject::WARNING, msg);
	 }

	 Database::getNextResultRow ();
      } // end-while has records
   } // endif has records

   TRACE9 ("StorageRecord::loadRecords () - Records: " << aRecords.size ());
   return Database::resultSize ();
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
   Database::execute (query.str ().c_str ());

   HSong song;
   while (Database::hasData ()) {
      song.define ();
      song->setId (Database::getResultColumnAsUInt (0));
      song->setName (Database::getResultColumnAsString (1));
      std::string time (Database::getResultColumnAsString (2));
      if (time != "00:00:00")
	 song->setDuration (time);
      song->setGenre (Database::getResultColumnAsUInt (3));
      unsigned int track (Database::getResultColumnAsUInt (4));
      if (track)
	 song->setTrack (track);

      songs.push_back (song);
      Database::getNextResultRow ();
   } // end-while
}

//-----------------------------------------------------------------------------
/// Saves the passed interpret.
/// \param interpret: Interpret to save
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveInterpret (const HInterpret interpret) throw (std::exception) {
   if (Storage::saveCelebrity (interpret))
      Database::execute ("INSERT INTO Interprets set id=LAST_INSERT_ID()");
}

//-----------------------------------------------------------------------------
/// Saves the passed record
/// \param record: Record to save
/// \param idInterpret: ID of interpret to which to record should be saved
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveRecord (const HRecord record, unsigned int idInterpret) throw (std::exception) {
   Check3 (idInterpret);

   std::stringstream query;
   query << (record->getId () ? "UPDATE Records" : "INSERT INTO Records")
	 << " SET name=\"" << Database::escapeDBValue (record->getName ())
	 << "\", interpret=" << idInterpret
	 << ", genre=" << record->getGenre ()
	 << ", year=" << (record->getYear ().isDefined () ? (unsigned int)record->getYear () : 0);
   if (record->getId ())
      query << " WHERE id=" << record->getId ();
   Database::execute (query.str ().c_str ());

   if (!record->getId ())
      record->setId (Database::getIDOfInsert ());
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveSong (const HSong song, unsigned int idRecord) throw (std::exception) {
   Check3 (idRecord);

   std::stringstream query;
   query << (song->getId () ? "UPDATE Songs" : "INSERT INTO Songs")
	 << " SET name=\"" << Database::escapeDBValue (song->getName ())
	 << "\", idRecord=" << idRecord
	 << ", duration=\"" << song->getDuration () << "\", genre="
	 << song->getGenre ()
	 << ", track=" << (song->getTrack ().isDefined () ? song->getTrack () : YGP::ANumeric (0));
   if (song->getId ())
      query << " WHERE id=" << song->getId ();
   Database::execute (query.str ().c_str ());

   if (!song->getId ())
      song->setId (Database::getIDOfInsert ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed song
/// \param idSongs: Song to delete
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteSong (unsigned int idSong) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Songs WHERE id=" << idSong;
   Database::execute (query.str ().c_str ());
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
   Database::execute (query.str ().c_str ());
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
   Database::execute (query.str ().c_str ());
}
