//$Id: StorageMovie.cpp,v 1.3 2006/02/27 20:45:35 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.3 $
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
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
#include "PMovies.h"

#include "StorageMovie.h"


//-----------------------------------------------------------------------------
/// Loads the movies from the database for a certain language.
/// \param directors: Vector of known directors
/// \param relMovies: Relation of above directors to their movies
/// \param lang: Language for movies to load
//-----------------------------------------------------------------------------
void StorageMovie::loadNames (const std::vector<HDirector>& directors,
			      const YGP::Relation1_N<HDirector, HMovie>& relMovies,
			      const std::string& lang) throw (std::exception) {
   std::string cmd ("SELECT id, name from MovieNames WHERE language='"
		    + lang + '\'');
   Database::execute (cmd.c_str ());

   while (Database::hasData ()) {
      TRACE9 ("PMovies::loadNames (const std::string&) - " << Database::getResultColumnAsUInt (0) << '/'
	      << Database::getResultColumnAsString (1));

      HMovie movie (PMovies::findMovie (directors, relMovies,
					Database::getResultColumnAsUInt (0)));
      Check3 (movie.isDefined ());
      movie->setName (Database::getResultColumnAsString (1), lang);
      Database::getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database.
/// \param aMovies: Map (with director-ID as index) to store the movies
/// \returns unsigned int: Number of loaded movies
//-----------------------------------------------------------------------------
unsigned int StorageMovie::loadMovies (std::map<unsigned int, std::vector<HMovie> >& aMovies,
			       YGP::StatusObject& stat) throw (std::exception) {
   Database::execute ("SELECT id, name, director, year, genre, type, languages"
		      ", subtitles FROM Movies ORDER BY director, year");
   if (Database::resultSize ()) {
      HMovie movie;
      unsigned int tmp;
      while (Database::hasData ()) {
	 // Fill and store movie entry from DB-values
	 TRACE8 ("PMovies::loadData () - Adding movie "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 try {
	    movie.define ();
	    movie->setName (Database::getResultColumnAsString (1), "");
	    movie->setId (Database::getResultColumnAsUInt (0));

	    tmp = Database::getResultColumnAsUInt (3);
	    if (tmp)
	       movie->setYear (tmp);
	    movie->setGenre (Database::getResultColumnAsUInt (4));
	    movie->setType (Database::getResultColumnAsUInt (5));
	    movie->setLanguage (Database::getResultColumnAsString (6));
	    movie->setTitles (Database::getResultColumnAsString (7));
	    aMovies[Database::getResultColumnAsUInt (2)].push_back (movie);
	 }
	 catch (std::exception& e) {
	    Glib::ustring msg (_("Warning loading movie `%1': %2"));
	    msg.replace (msg.find ("%1"), 2, movie->getName ());
	    msg.replace (msg.find ("%2"), 2, e.what ());
	    stat.setMessage (YGP::StatusObject::WARNING, msg);
	 }

	 Database::getNextResultRow ();
      } // end-while has movies
   } // endif movies found
   return Database::resultSize ();
}


//-----------------------------------------------------------------------------
/// Saves the passed director to the databas
/// \param director: Director to save
//-----------------------------------------------------------------------------
void StorageMovie::saveDirector (const HDirector director) throw (std::exception) {
   std::stringstream query;
   query << (director->getId () ? "UPDATE Celebrities" : "INSERT INTO Celebrities")
	 << " SET name=\"" << Database::escapeDBValue (director->getName ())
	 << "\", born="
	 << (director->getBorn ().isDefined () ? director->getBorn () : YGP::AYear (0))
	 << ", died="
	 << (director->getDied ().isDefined () ? director->getDied () : YGP::AYear (0));

   if (director->getId ())
      query << " WHERE id=" << director->getId ();
   Database::execute (query.str ().c_str ());

   if (!director->getId ()) {
      director->setId (Database::getIDOfInsert ());
      Database::execute ("INSERT INTO Directors set id=LAST_INSERT_ID()");
   }
}

//-----------------------------------------------------------------------------
/// Saves the passed movie to the databas
/// \param movie: Movie to save
/// \param idDirector: ID of director
//-----------------------------------------------------------------------------
void StorageMovie::saveMovie (const HMovie movie, unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << (movie->getId () ? "UPDATE Movies" : "INSERT INTO Movies")
	 << " SET name=\"" << Database::escapeDBValue (movie->getName (""))
	 << "\", genre=" << movie->getGenre () << ", languages=\""
	 << movie->getLanguage () << "\", subtitles=\""
	 << movie->getTitles () << "\", type=" << movie->getType ()
	 << ", director=" << idDirector;

   if (movie->getYear ().isDefined ())
      query << ", year=" << movie->getYear ();
   if (movie->getId ())
      query << " WHERE id=" << movie->getId ();

   Database::execute (query.str ().c_str ());
   if (!movie->getId ())
      movie->setId (Database::getIDOfInsert ());
}

//-----------------------------------------------------------------------------
/// Deletes all the names of the passed movie
/// \param idMovie: ID of movie whose (translated) names should be deleted
//-----------------------------------------------------------------------------
void StorageMovie::deleteMovieNames (unsigned int idMovie) throw (std::exception) {
   std::stringstream del;
   del << "DELETE FROM MovieNames WHERE id=" << idMovie;
   Database::execute (del.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Saves a (translated) name to a movie
/// \param movie: Movie to save
/// \param lang: Identification of the language
//-----------------------------------------------------------------------------
void StorageMovie::saveMovieName (const HMovie movie, const std::string& lang) throw (std::exception) {
   std::stringstream ins;
   ins << "INSERT INTO MovieNames SET id=" << movie->getId ()
       << ", name=\"" << Database::escapeDBValue (movie->getName (lang))
       << "\", language='" << lang << '\'';
   Database::execute (ins.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed director from the database
/// \param idDirector: ID of director to delete
//-----------------------------------------------------------------------------
void StorageMovie::deleteDirector (unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Directors WHERE id=" << idDirector;
   Database::execute (query.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed movie from the database
/// \param idMovie: ID of movie to delete
//-----------------------------------------------------------------------------
void StorageMovie::deleteMovie (unsigned int idMovie) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Movies WHERE id=" << idMovie;
   Database::execute (query.str ().c_str ());
}
