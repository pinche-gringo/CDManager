//$Id: StorageActor.cpp,v 1.1 2006/01/22 18:34:32 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 21.01.2006
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

#include "DB.h"

#include "StorageActor.h"


//-----------------------------------------------------------------------------
/// Loads the actors and its movies from the database
/// \param aActors: Map to map an actor-id to movie-IDs
//-----------------------------------------------------------------------------
void StorageActor::loadActorsInMovies (std::map<unsigned int, std::vector<unsigned int> >& aActors)
   throw (std::exception) {
   Database::execute ("SELECT idActor, idMovie FROM ActorsInMovies ORDER BY idMovie");
   if (Database::resultSize ()) {
      while (Database::hasData ()) {
	 aActors[Database::getResultColumnAsUInt (0)].push_back (Database::getResultColumnAsUInt (1));

	 Database::getNextResultRow ();
      } // end-while actors for movies available
   } // endif actors for movies stored in the DB
}

//-----------------------------------------------------------------------------
/// Saves the passed actor to the databas
/// \param actor: Actor to save
//-----------------------------------------------------------------------------
void StorageActor::saveActor (const HActor actor) throw (std::exception) {
   std::stringstream query;
   std::string tmp (Database::escapeDBValue (actor->getName ()));
   query << (actor->getId () ? "UPDATE Celebrities" : "INSERT INTO Celebrities")
	 << " SET name=\"" << tmp
	 << "\", born="
	 << (actor->getBorn ().isDefined () ? actor->getBorn () : YGP::AYear (0))
	 << ", died="
	 << (actor->getDied ().isDefined () ? actor->getDied () : YGP::AYear (0));

   if (actor->getId ())
      query << " WHERE id=" << actor->getId ();
   Database::execute (query.str ().c_str ());

   if (!actor->getId ()) {
      actor->setId (Database::getIDOfInsert ());
      Database::execute ("INSERT INTO Actors set id=LAST_INSERT_ID()");
   }
}

//-----------------------------------------------------------------------------
/// Deletes the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActor (unsigned int idActor) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Actors WHERE id=" << idActor;
   Database::execute (query.str ().c_str ());

   std::stringstream query2;
   query2 << "DELETE FROM ActorsInMovies WHERE id=" << idActor;
   Database::execute (query2.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes the movies of the actor with the passed ID from the database
/// \param idActor: Actor to remove
//-----------------------------------------------------------------------------
void StorageActor::deleteActorMovies (unsigned int idActor) throw (std::exception) {
   std::stringstream del;
   del << "DELETE FROM ActorsInMovies WHERE idActor=" << idActor;
   Database::execute (del.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Connects an actor with a movie
/// \param idActor: Actor to connect
/// \param idMovie: ID of movie the actors plays in
//-----------------------------------------------------------------------------
void StorageActor::saveActorMovie (unsigned int idActor, unsigned int idMovie) throw (std::exception) {
   std::stringstream query;
   query << "INSERT INTO ActorsInMovies SET idActor=" << idActor << ", idMovie=" << idMovie;
   Database::execute (query.str ().c_str ());
}
