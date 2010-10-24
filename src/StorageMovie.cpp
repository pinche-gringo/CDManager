//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
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
      TRACE9 ("StorageMovie::loadNames (const std::string&) - " << Database::getResultColumnAsUInt (0) << '/'
	      << Database::getResultColumnAsString (1));

      HMovie movie (PMovies::findMovie (directors, relMovies,
					Database::getResultColumnAsUInt (0)));
      Check3 (movie);
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
		      ", subtitles, summary, image FROM Movies ORDER BY director, year, name");
   if (Database::resultSize ()) {
      HMovie movie;
      unsigned int tmp;
      while (Database::hasData ()) {
	 // Fill and store movie entry from DB-values
	 TRACE8 ("StorageMovie::loadData () - Adding movie "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 try {
	    movie.reset (new Movie);
	    movie->setName (Database::getResultColumnAsString (1), "");
	    movie->setId (Database::getResultColumnAsUInt (0));

	    tmp = Database::getResultColumnAsUInt (3);
	    if (tmp)
	       movie->setYear (tmp);
	    movie->setGenre (Database::getResultColumnAsUInt (4));
	    movie->setType (Database::getResultColumnAsUInt (5));
	    movie->setLanguage (Database::getResultColumnAsString (6));
	    movie->setTitles (Database::getResultColumnAsString (7));
	    movie->setDescription (Database::getResultColumnAsString (8));
	    movie->setImage (Database::getResultColumnAsString (9));
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
/// Saves the passed movie to the databas
/// \param movie: Movie to save
/// \param idDirector: ID of director
//-----------------------------------------------------------------------------
void StorageMovie::saveMovie (const HMovie movie, unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << (movie->getId () ? "UPDATE" : "INSERT INTO")
	 << " Movies SET name=\"" << Database::escapeDBValue (movie->getName (""))
	 << "\", summary=\"" << Database::escapeDBValue (movie->getDescription ())
	 << "\", image=\"" << Database::escapeDBValue (movie->getImage ())
	 << "\", genre=" << movie->getGenre () << ", languages=\""
	 << movie->getLanguage () << "\", subtitles=\""
	 << movie->getTitles () << "\", type=" << movie->getType ()
	 << ", director=" << idDirector << ", year="
	 << (movie->getYear ().isDefined () ? (int)movie->getYear () : 0);
   if (movie->getId ())
      query << " WHERE id=" << movie->getId ();

   Database::execute (query.str ().c_str ());
   if (!movie->getId ())
      movie->setId (Database::getIDOfInsert ());

   const std::map<std::string, Glib::ustring>& names (movie->getNames ()); Check3 (names.begin () != names.end ());
   for (std::map<std::string, Glib::ustring>::const_iterator i (names.begin ());
	++i != names.end ();)
      saveMovieName (movie, i->first);
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
/// Saves a (translated) name to a movie. Entries with empty name are deleted
/// \param movie: Movie to save
/// \param lang: Identification of the language
//-----------------------------------------------------------------------------
void StorageMovie::saveMovieName (const HMovie movie, const std::string& lang) throw (std::exception) {
   std::stringstream cmd;
   if (movie->getName (lang).size ()) {
      cmd << "INSERT INTO MovieNames SET id=" << movie->getId ()
	  << ", name=\"" << Database::escapeDBValue (movie->getName (lang))
	  << "\", language='" << lang << '\'';
      try {
	 Database::execute (cmd.str ().c_str ());
      }
      catch (...) {
	 std::stringstream upd;
	 upd << "UPDATE MovieNames SET name=\"" << Database::escapeDBValue (movie->getName (lang)) << '"'
	     << " WHERE id=" << movie->getId () << " AND language='" << lang << '\'';
	 Database::execute (upd.str ().c_str ());
      }
   }
   else {
      cmd << "DELETE FROM MovieNames" << " WHERE id=" << movie->getId () << " AND language='" << lang << '\'';
      Database::execute (cmd.str ().c_str ());
   }
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

   deleteMovieNames (idMovie);
}
