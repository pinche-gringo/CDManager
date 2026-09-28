// PROJECT     : CDManager
// SUBSYSTEM   : Storage
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 24.01.2006
// COPYRIGHT   : Copyright (C) 2006, 2009, 2010, 2026

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

#include <format>
#include <memory>

#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include "DB.h"

#include "StorageRecord.h"

//-----------------------------------------------------------------------------
/// Loads the records from the database.
/// \param aRecords: Map (with interpret-ID as index) to store the records
/// \returns unsigned int: Number of loaded records
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
unsigned int StorageRecord::loadRecords(std::map<unsigned int, std::vector<HRecord>>& aRecords, YGP::StatusObject& stat) {
    db().execute("SELECT id, name, interpret, year, genre FROM "
                 "Records ORDER BY interpret, year");
    TRACE8("StorageRecord::loadRecords () - Records: " << db().resultSize());

    if (db().resultSize()) {
        HRecord newRec;
        while (db().hasData()) {
            // Fill and store record entry from DB-values
            TRACE8("StorageRecords::loadRecords (...) - Adding record " << db().getResultColumnAsUInt(0) << '/'
                                                                        << db().getResultColumnAsString(1));
            newRec = std::make_shared<Record>();

            try {
                newRec->setId(db().getResultColumnAsUInt(0));
                newRec->setName(db().getResultColumnAsString(1));
                if (db().getResultColumnAsUInt(3))
                    newRec->setYear(db().getResultColumnAsUInt(3));
                newRec->setGenre(db().getResultColumnAsUInt(4));

                aRecords[db().getResultColumnAsUInt(2)].push_back(newRec);
            }
            catch (const std::exception& e) {
                stat.setMessage(YGP::StatusObject::WARNING,
                                Glib::ustring::compose(_("Warning loading record `%1': %2"), newRec->getName(), e.what()));
            }

            db().getNextResultRow();
        } // end-while has records
    } // endif has records

    TRACE9("StorageRecord::loadRecords () - Records: " << aRecords.size());
    return db().resultSize();
}

//-----------------------------------------------------------------------------
/// Loads the songs for a record from the database
/// \param idRecord: ID of record whose songs shall be loaded
/// \param songs: Vector to store loaded songs
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::loadSongs(unsigned int idRecord, std::vector<HSong>& songs) {
    TRACE9("StorageRecord::loadSongs (unsigned int, std::vector<HSong>&) - " << idRecord);

    db().execute(std::format("SELECT id, name, duration, genre, track FROM Songs WHERE idRecord={}", idRecord));

    HSong song;
    while (db().hasData()) {
        song = std::make_shared<Song>();
        song->setId(db().getResultColumnAsUInt(0));
        song->setName(db().getResultColumnAsString(1));
        if (const std::string time(db().getResultColumnAsString(2)); time != "00:00:00")
            song->setDuration(time);
        song->setGenre(db().getResultColumnAsUInt(3));
        if (const unsigned int track(db().getResultColumnAsUInt(4)); track)
            song->setTrack(track);

        songs.push_back(song);
        db().getNextResultRow();
    } // end-while
}

//-----------------------------------------------------------------------------
/// Saves the passed record
/// \param record: Record to save
/// \param idInterpret: ID of interpret to which to record should be saved
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveRecord(const HRecord record, unsigned int idInterpret) {
    Database::Values values;
    values("name", db().quote(record->getName()))("interpret", idInterpret)("genre", record->getGenre())(
        "year", record->getYear().isDefined() ? static_cast<unsigned int>(record->getYear()) : 0);

    if (record->getId()) {
        db().update("Records", values, std::format("id={}", record->getId()));
    }
    else {
        db().insert("Records", values);
        record->setId(db().getIDOfInsert());
    }
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::saveSong(const HSong song, unsigned int idRecord) {
    Database::Values values;
    values("name", db().quote(song->getName()))("idRecord", idRecord)("duration",
                                                                      db().quote(song->getDuration().toUnformattedString()))(
        "genre", song->getGenre())("track", song->getTrack().isDefined() ? song->getTrack() : YGP::ANumeric(0));

    if (song->getId()) {
        db().update("Songs", values, std::format("id={}", song->getId()));
    }
    else {
        db().insert("Songs", values);
        song->setId(db().getIDOfInsert());
    }
}

//-----------------------------------------------------------------------------
/// Deletes the passed song
/// \param idSongs: Song to delete
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteSong(unsigned int idSong) {
    db().execute(std::format("DELETE FROM Songs WHERE id={}", idSong));
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteRecord(unsigned int idRecord) {
    db().execute(std::format("DELETE FROM Records WHERE id={}", idRecord));
}

//-----------------------------------------------------------------------------
/// Saves the passed song
/// \param songs: Song to save
/// \param idRecord: ID of record to song belongs to
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void StorageRecord::deleteInterpret(unsigned int idInterpret) {
    db().execute(std::format("DELETE FROM Interprets WHERE id={}", idInterpret));
}
