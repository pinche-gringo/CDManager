// PROJECT     : CDManager
// SUBSYSTEM   : Storage
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 21.01.2006
// COPYRIGHT   : Copyright (C) 2006, 2010, 2011, 2026

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

#include <format>

#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include "DB.h"

#include "StorageActor.h"

//-----------------------------------------------------------------------------
/// Loads the actors and its films from the database
/// \param aActors: Map to map an actor-id to film-IDs
//-----------------------------------------------------------------------------
void StorageActor::loadActorsInFilms(std::map<unsigned int, std::vector<unsigned int>>& aActors) {
    TRACE7("StorageActor::loadActorsInFilms (std::map<...>&)");

    db().execute("SELECT idActor, idFilm FROM ActorsInFilms ORDER BY idActor");
    if (db().resultSize()) {
        auto iter(aActors.end());
        while (db().hasData()) {
            unsigned int idLast(0);
            const unsigned int idAct(db().getResultColumnAsUInt(0));
            contract_assert(idAct);
            if (idAct != idLast) {
                idLast = idAct;
                iter = aActors.try_emplace(aActors.end(), idAct);
            }
            contract_assert(iter != aActors.end());
            iter->second.push_back(db().getResultColumnAsUInt(1));

            db().getNextResultRow();
        } // end-while actors for films available
    } // endif actors for films stored in the DB
}

//-----------------------------------------------------------------------------
/// Deletes the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActor(unsigned int idActor) {
    TRACE9("StorageActor::deleteActor (unsigned int)");

    db().execute(std::format("DELETE FROM Actors WHERE id={}", idActor));
    db().execute(std::format("DELETE FROM ActorsInFilms WHERE idActor={}", idActor));
}

//-----------------------------------------------------------------------------
/// Deletes the films of the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActorFilms(unsigned int idActor) {
    TRACE9("StorageActor::deleteActorFilms (unsigned int)");

    db().execute(std::format("DELETE FROM ActorsInFilms WHERE idActor={}", idActor));
}

//-----------------------------------------------------------------------------
/// Connects an actor with a film
/// \param idActor: Actor to connect
/// \param idFilm: ID of film the actors plays in
//-----------------------------------------------------------------------------
void StorageActor::saveActorFilm(unsigned int idActor, unsigned int idFilm) {
    db().execute(std::format("INSERT INTO ActorsInFilms (idActor, idFilm) VALUES ({}, {})", idActor, idFilm));
}
