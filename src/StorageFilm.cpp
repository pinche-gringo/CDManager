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
   db ().execute ("SELECT id, name from FilmNames WHERE language=" + db ().quote (lang));

   while (db ().hasData ()) {
      TRACE9 ("StorageFilm::loadNames (const std::string&) - " << db ().getResultColumnAsUInt (0) << '/'
	      << db ().getResultColumnAsString (1));

      HFilm film (PFilms::findFilm (directors, relFilms,
					db ().getResultColumnAsUInt (0)));
      Check3 (film);
      film->setName (db ().getResultColumnAsString (1), lang);
      db ().getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the films from the database.
/// \param aFilms: Map (with director-ID as index) to store the films
/// \returns unsigned int: Number of loaded films
//-----------------------------------------------------------------------------
unsigned int StorageFilm::loadFilms (std::map<unsigned int, std::vector<HFilm> >& aFilms,
				     YGP::StatusObject& stat) throw (std::exception) {
   db ().execute ("SELECT id, name, director, year, genre, type, languages"
		      ", subtitles, summary, image FROM Films ORDER BY director, year, name");
   if (db ().resultSize ()) {
      HFilm film;
      unsigned int tmp;
      while (db ().hasData ()) {
	 // Fill and store film entry from DB-values
	 TRACE8 ("StorageFilm::loadData () - Adding film "
		 << db ().getResultColumnAsUInt (0) << '/'
		 << db ().getResultColumnAsString (1));

	 try {
	    film.reset (new Film);
	    film->setName (db ().getResultColumnAsString (1), "");
	    film->setId (db ().getResultColumnAsUInt (0));

	    tmp = db ().getResultColumnAsUInt (3);
	    if (tmp)
	       film->setYear (tmp);
	    film->setGenre (db ().getResultColumnAsUInt (4));
	    film->setType (db ().getResultColumnAsUInt (5));
	    film->setLanguage (db ().getResultColumnAsString (6));
	    film->setTitles (db ().getResultColumnAsString (7));
	    film->setDescription (db ().getResultColumnAsString (8));
	    film->setImage (db ().getResultColumnAsBlob (9));
	    aFilms[db ().getResultColumnAsUInt (2)].push_back (film);
	 }
	 catch (std::exception& e) {
	    Glib::ustring msg (_("Warning loading film `%1': %2"));
	    msg.replace (msg.find ("%1"), 2, film->getName ());
	    msg.replace (msg.find ("%2"), 2, e.what ());
	    stat.setMessage (YGP::StatusObject::WARNING, msg);
	 }

	 db ().getNextResultRow ();
      } // end-while has films
   } // endif films found
   return db ().resultSize ();
}

//-----------------------------------------------------------------------------
/// Saves the passed film to the databas
/// \param film: Film to save
/// \param idDirector: ID of director
//-----------------------------------------------------------------------------
void StorageFilm::saveFilm (const HFilm film, unsigned int idDirector) throw (std::exception) {
   Database::Values values;
   values ("name", db ().quote (film->getName ("")))
      ("summary", db ().quote (film->getDescription ()))
      ("image", db ().quoteBlob (film->getImage ()))
      ("genre", film->getGenre ())
      ("languages", db ().quote (film->getLanguage ()))
      ("subtitles", db ().quote (film->getTitles ()))
      ("type", film->getType ())
      ("director", idDirector)
      ("year", film->getYear ().isDefined () ? (int)film->getYear () : 0);

   if (film->getId ()) {
      std::stringstream where;
      where << "id=" << film->getId ();
      db ().update ("Films", values, where.str ());
   }
   else {
      db ().insert ("Films", values);
      film->setId (db ().getIDOfInsert ());
   }

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
   db ().execute (del.str ());
}

//-----------------------------------------------------------------------------
/// Saves a (translated) name to a film. Entries with empty name are deleted
/// \param film: Film to save
/// \param lang: Identification of the language
//-----------------------------------------------------------------------------
void StorageFilm::saveFilmName (const HFilm film, const std::string& lang) throw (std::exception) {
   std::stringstream where;
   where << "id=" << film->getId () << " AND language=" << db ().quote (lang);

   if (film->getName (lang).size ()) {
      // Check for an existing entry (instead of catching the failing INSERT),
      // as a failing statement aborts the whole transaction in PostgreSQL
      db ().execute ("SELECT id FROM FilmNames WHERE " + where.str ());

      Database::Values values;
      values ("name", db ().quote (film->getName (lang)));
      if (db ().hasData ())
	 db ().update ("FilmNames", values, where.str ());
      else {
	 values ("id", film->getId ()) ("language", db ().quote (lang));
	 db ().insert ("FilmNames", values);
      }
   }
   else
      db ().execute ("DELETE FROM FilmNames WHERE " + where.str ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed director from the database
/// \param idDirector: ID of director to delete
//-----------------------------------------------------------------------------
void StorageFilm::deleteDirector (unsigned int idDirector) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Directors WHERE id=" << idDirector;
   db ().execute (query.str ());
}

//-----------------------------------------------------------------------------
/// Deletes the passed film from the database
/// \param idFilm: ID of film to delete
//-----------------------------------------------------------------------------
void StorageFilm::deleteFilm (unsigned int idFilm) throw (std::exception) {
   std::stringstream query;
   query << "DELETE FROM Films WHERE id=" << idFilm;
   db ().execute (query.str ());

   deleteFilmNames (idFilm);
}
