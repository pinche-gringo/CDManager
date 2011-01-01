//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 21.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2010, 2011

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


#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include "DB.h"

#include "StorageActor.h"


//-----------------------------------------------------------------------------
/// Loads the actors and its films from the database
/// \param aActors: Map to map an actor-id to film-IDs
//-----------------------------------------------------------------------------
void StorageActor::StorageActor::loadActorsInFilms (std::map<unsigned int, std::vector<unsigned int> >& aActors)
   throw (std::exception) {
   TRACE7 ("StorageActor::loadActorsInFilms (std::map<...>&)");

   Database::execute ("SELECT idActor, idFilm FROM ActorsInFilms ORDER BY idActor");
   if (Database::resultSize ()) {
      std::map<unsigned int, std::vector<unsigned int> >::iterator iter (aActors.end ());
      while (Database::hasData ()) {
	 unsigned int idLast (0);
	 unsigned int idAct (Database::getResultColumnAsUInt (0)); Check3 (idAct);
	 if (idAct != idLast) {
	    idLast = idAct;
	    iter = aActors.insert (aActors.end (), std::pair<unsigned int, std::vector<unsigned int> > (idAct, std::vector<unsigned int> ()));
	 }
	 Check3 (iter != aActors.end ());
	 iter->second.push_back (Database::getResultColumnAsUInt (1));

	 Database::getNextResultRow ();
      } // end-while actors for films available
   } // endif actors for films stored in the DB
}

//-----------------------------------------------------------------------------
/// Deletes the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActor (unsigned int idActor) throw (std::exception) {
   TRACE9 ("StorageActor::deleteActor (unsigned int)");
   Check3 (idActor);

   std::stringstream query;
   query << "DELETE FROM Actors WHERE id=" << idActor;
   Database::execute (query.str ().c_str ());

   std::stringstream query2;
   query2 << "DELETE FROM ActorsInFilms WHERE idActor=" << idActor;
   Database::execute (query2.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes the films of the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActorFilms (unsigned int idActor) throw (std::exception) {
   TRACE9 ("StorageActor::deleteActorFilms (unsigned int)");
   Check3 (idActor);

   std::stringstream del;
   del << "DELETE FROM ActorsInFilms WHERE idActor=" << idActor;
   Database::execute (del.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Connects an actor with a film
/// \param idActor: Actor to connect
/// \param idFilm: ID of film the actors plays in
//-----------------------------------------------------------------------------
void StorageActor::saveActorFilm (unsigned int idActor, unsigned int idFilm) throw (std::exception) {
   std::stringstream query;
   query << "INSERT INTO ActorsInFilms SET idActor=" << idActor << ", idFilm=" << idFilm;
   Database::execute (query.str ().c_str ());
}
