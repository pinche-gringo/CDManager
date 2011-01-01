//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 22.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009 - 2011

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
#include "PFilms.h"

#include "StorageFilm.h"


//-----------------------------------------------------------------------------
/// Loads the films from the database for a certain language.
/// \param directors: Vector of known directors
/// \param relFilms: Relation of above directors to their films
/// \param lang: Language for films to load
//-----------------------------------------------------------------------------
void StorageFilm::loadNames (const std::vector<HDirector>& directors,
			      const YGP::Relation1_N<HDirector, HFilm>& relFilms,
			      const std::string& lang) throw (std::exception) {
   std::string cmd ("SELECT id, name from FilmNames WHERE language='"
		    + lang + '\'');
   Database::execute (cmd.c_str ());

   while (Database::hasData ()) {
      TRACE9 ("StorageFilm::loadNames (const std::string&) - " << Database::getResultColumnAsUInt (0) << '/'
	      << Database::getResultColumnAsString (1));

      HFilm film (PFilms::findFilm (directors, relFilms,
					Database::getResultColumnAsUInt (0)));
      Check3 (film);
      film->setName (Database::getResultColumnAsString (1), lang);
      Database::getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the films from the database.
/// \param aFilms: Map (with director-ID as index) to store the films
/// \returns unsigned int: Number of loaded films
//-----------------------------------------------------------------------------
unsigned int StorageFilm::loadFilms (std::map<unsigned int, std::vector<HFilm> >& aFilms,
			       YGP::StatusObject& stat) throw (std::exception) {
   Database::execute ("SELECT id, name, director, year, genre, type, languages"
		      ", subtitles, summary, image FROM Films ORDER BY director, year, name");
   if (Database::resultSize ()) {
      HFilm film;
      unsigned int tmp;
      while (Database::hasData ()) {
	 // Fill and store film entry from DB-values
	 TRACE8 ("StorageFilm::loadData () - Adding film "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 try {
	    film.reset (new Film);
	    film->setName (Database::getResultColumnAsString (1), "");
	    film->setId (Database::getResultColumnAsUInt (0));

	    tmp = Database::getResultColumnAsUInt (3);
	    if (tmp)
	       film->setYear (tmp);
	    film->setGenre (Database::getResultColumnAsUInt (4));
	    film->setType (Database::getResultColumnAsUInt (5));
	    film->setLanguage (Database::getResultColumnAsString (6));
	    film->setTitles (Database::getResultColumnAsString (7));
	    film->setDescription (Database::getResultColumnAsString (8));
	    film->setImage (Database::getResultColumnAsString (9));
	    aFilms[Database::getResultColumnAsUInt (2)].push_back (film);
	 }
	 catch (std::exception& e) {
	    Glib::ustring msg (_("Warning loading film `%1': %2"));
	    msg.replace (msg.find ("%1"), 2, film->getName ());
	    msg.replace (msg.find ("%2"), 2, e.what ());
	    stat.setMessage (YGP::StatusObject::WARNING, msg);
	 }

	 Database::getNextResultRow ();
      } // end-while has films
   } // endif films found
   return Database::resultSize ();
}

//-----------------------------------------------------------------------------
/// Saves the passed film to the databas
/// \param film: Film to save
/// \param idDirector: ID of director
//-----------------------------------------------------------------------------
void StorageFilm::saveFilm (const HFilm film, unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << (film->getId () ? "UPDATE" : "INSERT INTO")
	 << " Films SET name=\"" << Database::escapeDBValue (film->getName (""))
	 << "\", summary=\"" << Database::escapeDBValue (film->getDescription ())
	 << "\", image=\"" << Database::escapeDBValue (film->getImage ())
	 << "\", genre=" << film->getGenre () << ", languages=\""
	 << film->getLanguage () << "\", subtitles=\""
	 << film->getTitles () << "\", type=" << film->getType ()
	 << ", director=" << idDirector << ", year="
	 << (film->getYear ().isDefined () ? (int)film->getYear () : 0);
   if (film->getId ())
      query << " WHERE id=" << film->getId ();

   Database::execute (query.str ().c_str ());
   if (!film->getId ())
      film->setId (Database::getIDOfInsert ());

   const std::map<std::string, Glib::ustring>& names (film->getNames ()); Check3 (names.begin () != names.end ());
   for (std::map<std::string, Glib::ustring>::const_iterator i (names.begin ());
	++i != names.end ();)
      saveFilmName (film, i->first);
}

//-----------------------------------------------------------------------------
/// Deletes all the names of the passed film
/// \param idFilm: ID of film whose (translated) names should be deleted
//-----------------------------------------------------------------------------
void StorageFilm::deleteFilmNames (unsigned int idFilm) throw (std::exception) {
   std::stringstream del;
   del << "DELETE FROM FilmNames WHERE id=" << idFilm;
   Database::execute (del.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Saves a (translated) name to a film. Entries with empty name are deleted
/// \param film: Film to save
/// \param lang: Identification of the language
//-----------------------------------------------------------------------------
void StorageFilm::saveFilmName (const HFilm film, const std::string& lang) throw (std::exception) {
   std::stringstream cmd;
   if (film->getName (lang).size ()) {
      cmd << "INSERT INTO FilmNames SET id=" << film->getId ()
	  << ", name=\"" << Database::escapeDBValue (film->getName (lang))
	  << "\", language='" << lang << '\'';
      try {
	 Database::execute (cmd.str ().c_str ());
      }
      catch (...) {
	 std::stringstream upd;
	 upd << "UPDATE FilmNames SET name=\"" << Database::escapeDBValue (film->getName (lang)) << '"'
	     << " WHERE id=" << film->getId () << " AND language='" << lang << '\'';
	 Database::execute (upd.str ().c_str ());
      }
   }
   else {
      cmd << "DELETE FROM FilmNames" << " WHERE id=" << film->getId () << " AND language='" << lang << '\'';
      Database::execute (cmd.str ().c_str ());
   }
}

//-----------------------------------------------------------------------------
/// Deletes the passed director from the database
/// \param idDirector: ID of director to delete
//-----------------------------------------------------------------------------
void StorageFilm::deleteDirector (unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Directors WHERE id=" << idDirector;
   Database::execute (query.str ().c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed film from the database
/// \param idFilm: ID of film to delete
//-----------------------------------------------------------------------------
void StorageFilm::deleteFilm (unsigned int idFilm) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Films WHERE id=" << idFilm;
   Database::execute (query.str ().c_str ());

   deleteFilmNames (idFilm);
}
