//$Id: CDManagerDB.cpp,v 1.15 2006/01/20 00:31:11 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.15 $
//AUTHOR      : Markus Schwab
//CREATED     : 24.1.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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

#include <cstdlib>
#include <clocale>

#include <fstream>
#include <sstream>

#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>
#include <YGP/Tokenize.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include "DB.h"
#include "Words.h"

#include "CDManager.h"


//-----------------------------------------------------------------------------
/// Finds the movie with the passed id
/// \param id: Id of movie to find
/// \returns HMovie: Found movie (undefined, if not found)
//-----------------------------------------------------------------------------
HMovie CDManager::findMovie (unsigned int id) const {
   for (std::vector<HDirector>::const_iterator d (directors.begin ());
	d != directors.end (); ++d) {
      Check3 (d->isDefined ());
      const std::vector<HMovie>& movies (relMovies.getObjects (*d));

      for (std::vector<HMovie>::const_iterator m (movies.begin ()); m != movies.end (); ++m) {
	 Check3 (m->isDefined ());
	 if ((*m)->getId () == id)
	    return *m;
	 }
      }
   HMovie m;
   return m;
}

//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param user: User to connect to the DB with
/// \param pwd: Password for user
/// \returns bool: True, if login could be performed
//-----------------------------------------------------------------------------
bool CDManager::login (const Glib::ustring& user, const Glib::ustring& pwd) {
   TRACE9 ("CDManager::login (const Glib::ustring&, const Glib::ustring&) - " << opt << '/' << pwd);

   try {
      Database::connect (DBNAME, user.c_str (), pwd.c_str ());
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't connect to database!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Login error"));
      dlg.run ();

      showLogin ();
      return false;
   }
   TRACE9 ("CDManager::login (const Glib::ustring&, const Glib::ustring&) - Logged in");

   enableMenus (true);

   loadedPages = 0;
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &CDManager::loadDatabase), false));

   try {
      if (recGenres.empty ()) {
	 const char* pLang (getenv ("LANGUAGE"));
	 if (!pLang) {
#ifdef HAVE_LC_MESSAGES
	    pLang = setlocale (LC_MESSAGES, NULL);
#else
	    pLang = getenv ("LANG");
#endif
	 }
	 Genres::loadFromFile (DATADIR "Genres.dat", recGenres, movieGenres, pLang);

	 records.updateGenres ();
	 songs.updateGenres ();
	 movies.updateGenres ();
      }

      Words::create ();
      if (!Words::cArticles ()) {
	 Database::execute ("SELECT word FROM Words");
	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    Words::addName2Ignore (Database::getResultColumnAsString (0), Words::POS_END);
	    Database::getNextResultRow ();
	 }

	 Database::execute ("SELECT article FROM Articles");
	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    Words::addArticle (Database::getResultColumnAsString (0), Words::POS_END);
	    Database::getNextResultRow ();
	 }
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query needed information!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Logout from the DB; give an opportunity to save changes
//-----------------------------------------------------------------------------
void CDManager::logout () {
   on_delete_event (NULL);

   // Cleanup undo-info
   relDelActors.unrelateAll ();
   undoEntities.erase (undoEntities.begin (), undoEntities.end ());

   Words::destroy ();
   Database::close ();
   enableMenus (false);
   status.pop ();
   status.push (_("Disconnected!"));
}

//-----------------------------------------------------------------------------
/// Loads the stored celebrities from the database
/// \param target: Vector, in which to load the celebrities
/// \param table: Database table from which to load them
/// \param stat: Statusobject, in which to return the errors
//-----------------------------------------------------------------------------
void CDManager::loadCelebrities (std::vector<HCelebrity>& target, const std::string& table,
				 YGP::StatusObject& stat) {
   TRACE9 ("CDManager::loadCelebrities (std::vector<HCelebrity>&, const std::string&,\n\tYGP::StatusObject&) - " << table);

   // Load data from movies table
   std::string cmd ("SELECT c.id, c.name, c.born, c.died FROM Celebrities c, ");
   cmd += table;
   cmd += " x WHERE c.id = x.id";
   Database::execute (cmd.c_str ());

   HCelebrity hCeleb;
   while (Database::hasData ()) {
      TRACE5 ("CDManager::laodMovies () - Adding " << Database::getResultColumnAsUInt (0) << '/' << Database::getResultColumnAsString (1));

      // Fill and store entry from DB-values
      try {
	 hCeleb.define ();
	 hCeleb->setId (Database::getResultColumnAsUInt (0));
	 hCeleb->setName (Database::getResultColumnAsString (1));

	 std::string tmp (Database::getResultColumnAsString (2));
	 if (tmp != "0000")
	    hCeleb->setBorn (tmp);
	 tmp = Database::getResultColumnAsString (3);
	 if (tmp != "0000")
	    hCeleb->setDied (tmp);
      }
      catch (std::exception& e) {
	 Glib::ustring msg (_("Warning loading celebrity `%1': %2"));
	 msg.replace (msg.find ("%1"), 2, hCeleb->getName ());
	 msg.replace (msg.find ("%2"), 2, e.what ());
	 stat.setMessage (YGP::StatusObject::WARNING, msg);
      }
      target.push_back (hCeleb);

      Database::getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the records from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadRecords () {
   TRACE9 ("CDManager::loadRecords ()");
   try {
      unsigned long int cRecords (0);
      YGP::StatusObject stat;
      loadCelebrities (artists, "Interprets", stat);
      std::sort (artists.begin (), artists.end (), &Interpret::compByName);

      Database::execute ("SELECT id, name, interpret, year, genre FROM "
			 "Records ORDER BY interpret, year");
      TRACE8 ("CDManager::loadRecords () - Records: " << Database::resultSize ());

      if ((cRecords = Database::resultSize ())) {
	 std::map<unsigned int, std::vector<HRecord> > aRecords;

	 HRecord newRec;
	 while (Database::hasData ()) {
	    // Fill and store record entry from DB-values
	    TRACE8 ("CDManager::loadRecords () - Adding record "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	    newRec.define ();
	    newRec->setId (Database::getResultColumnAsUInt (0));
	    newRec->setName (Database::getResultColumnAsString (1));
	    if (Database::getResultColumnAsUInt (3))
	       newRec->setYear (Database::getResultColumnAsUInt (3));
	    newRec->setGenre (Database::getResultColumnAsUInt (4));
	    aRecords[Database::getResultColumnAsUInt (2)].push_back (newRec);
	    Database::getNextResultRow ();
	 } // end-while has records

	 for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	      i != artists.end (); ++i) {
	    Gtk::TreeModel::Row artist (records.append (*i));

	    std::map<unsigned int, std::vector<HRecord> >::iterator iRec
	       (aRecords.find ((*i)->getId ()));
	    if (iRec != aRecords.end ()) {
	       for (std::vector<HRecord>::iterator r (iRec->second.begin ());
		    r != iRec->second.end (); ++r) {
		  records.append (*r, artist);
		  relRecords.relate (*i, *r);
	       } // end-for all records for an artist
	       aRecords.erase (iRec);
	    } // end-if artist has record
	 } // end-for all artists
	 records.expand_all ();
      } // end-if database contains records

      loadedPages |= 1;

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 record", "Loaded %1 records", cRecords)));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cRecords));

      Glib::ustring tmp (Glib::locale_to_utf8 (ngettext (" from %1 artist", " from %1 artists", artists.size ())));
      tmp.replace (tmp.find ("%1"), 2, YGP::ANumeric::toString (artists.size ()));
      msg += tmp;
      status.pop ();
      status.push (msg);

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading movies"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available records!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database for a certain language.
//-----------------------------------------------------------------------------
void CDManager::loadMovies (const std::string& lang) {
   TRACE5 ("CDManager::loadMovies (const std::string&) - Language: " << lang);
   try {
      std::string cmd ("SELECT id, name from MovieNames WHERE language='"
		       + lang + '\'');
      Database::execute (cmd.c_str ());

      while (Database::hasData ()) {
	 TRACE9 ("CDManager::loadMovies (const std::string&) - " << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 HMovie movie (findMovie (Database::getResultColumnAsUInt (0))); Check3 (movie.isDefined ());
	 movie->setName (Database::getResultColumnAsString (1), lang);
	 Database::getNextResultRow ();
      }
      loadedLangs[lang] = true;
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database.
//-----------------------------------------------------------------------------
void CDManager::loadMovies () {
   TRACE9 ("CDManager::loadMovies ()");
   try {
      unsigned int cMovies (0);

      YGP::StatusObject stat;
      loadCelebrities (directors, "Directors", stat);
      std::sort (directors.begin (), directors.end (), &Director::compByName);

      Database::execute ("SELECT id, name, director, year, genre, type, languages"
			 ", subtitles FROM Movies ORDER BY director, year");
      TRACE8 ("CDManager::loadMovies () - Found " << Database::resultSize ()
	      << " movies");

      if ((cMovies = Database::resultSize ())) {
	 std::map<unsigned int, std::vector<HMovie> > aMovies;

	 HMovie movie;
	 while (Database::hasData ()) {
	    // Fill and store movie entry from DB-values
	    TRACE8 ("CDManager::loadMovies () - Adding movie "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	    try {
	       movie.define ();
	       movie->setName (Database::getResultColumnAsString (1), "");
	       movie->setId (Database::getResultColumnAsUInt (0));
	       if (Database::getResultColumnAsUInt (3))
		  movie->setYear (Database::getResultColumnAsUInt (3));
	       movie->setGenre (Database::getResultColumnAsUInt (4));
	       movie->setType (Database::getResultColumnAsUInt (5));
	       movie->setLanguage (Database::getResultColumnAsString (6));
	       movie->setTitles (Database::getResultColumnAsString (7));
	    }
	    catch (std::exception& e) {
	       Glib::ustring msg (_("Warning loading movie `%1': %2"));
	       msg.replace (msg.find ("%1"), 2, movie->getName ());
	       msg.replace (msg.find ("%2"), 2, e.what ());
	       stat.setMessage (YGP::StatusObject::WARNING, msg);
	    }

	    aMovies[Database::getResultColumnAsUInt (2)].push_back (movie);
	    Database::getNextResultRow ();
	 } // end-while has movies

	 for (std::vector<HDirector>::const_iterator i (directors.begin ());
	      i != directors.end (); ++i) {
	    Check3 (i->isDefined ());
	    Gtk::TreeModel::Row director (movies.append (*i));

	    std::map<unsigned int, std::vector<HMovie> >::iterator iMovie
	       (aMovies.find ((*i)->getId ()));
	    if (iMovie != aMovies.end ()) {
	       for (std::vector<HMovie>::iterator m (iMovie->second.begin ());
		    m != iMovie->second.end (); ++m) {
		  movies.append (*m, director);
		  relMovies.relate (*i, *m);
	       } // end-for all movies for a director
	       aMovies.erase (iMovie);
	    } // end-if director has movies
	 } // end-for all directors
      } // end-if database contains movies

      movies.expand_all ();

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 movie", "Loaded %1 movies", cMovies)));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cMovies));
      status.pop ();
      status.push (msg);

      loadedPages |= 2;

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading movies"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the songs for the passed record
/// \param record: Handle to the record for which to load songs
//-----------------------------------------------------------------------------
void CDManager::loadSongs (const HRecord& record) {
   TRACE9 ("CDManager::loadSongs (const HRecord& record) - "
	   << (record.isDefined () ? record->getName ().c_str () : "Undefined"));
   Check1 (record.isDefined ());

   try {
      std::stringstream query;
      query << "SELECT id, name, duration, genre, track FROM Songs WHERE"
	 " idRecord=" << record->getId ();
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

	 relSongs.relate (record, song);
	 Database::getNextResultRow ();
      } // end-while

      record->setSongsLoaded ();
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the songs for record %1!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, record->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the actors from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadActors () {
   TRACE9 ("CDManager::loadActors ()");

   try {
      YGP::StatusObject stat;
      std::vector<HActor> actrs;
      loadCelebrities (actrs, "Actors", stat);

      if (actrs.size ()) {
	 std::sort (actrs.begin (), actrs.end (), &Actor::compById);

	 Database::execute ("SELECT idActor, idMovie FROM ActorsInMovies ORDER BY idMovie");
	 std::map<unsigned int, std::vector<HMovie> > aActors;
	 if (Database::resultSize ()) {
	    HMovie movie; movie.define ();
	    while (Database::hasData ()) {
	       unsigned int idMovie (Database::getResultColumnAsUInt (1));
	       if (idMovie != movie->getId ())
		  movie = findMovie (idMovie);

	       Check3 (movie.isDefined ());
	       Check3 (movie->getId ());
	       aActors[Database::getResultColumnAsUInt (0)].push_back (movie);

	       Database::getNextResultRow ();
	    } // end-while actors for movies available
	 } // endif actors for movies stored in the DB

	 std::sort (actrs.begin (), actrs.end (), &Actor::compByName);
	 for (std::vector<HActor>::const_iterator i (actrs.begin ()); i != actrs.end (); ++i) {
	    Check3 (i->isDefined ());
	    Gtk::TreeModel::Row actor (actors.append (*i));

	    std::map<unsigned int, std::vector<HMovie> >::iterator iActor
	       (aActors.find ((*i)->getId ()));
	    if (iActor != aActors.end ()) {
	       std::sort (iActor->second.begin (), iActor->second.end (), &Movie::compByName);
	       for (std::vector<HMovie>::iterator m (iActor->second.begin ());
		    m != iActor->second.end (); ++m) {
		  actors.append (*m, actor);
		  relActors.relate (*i, *m);
	       } // end-for all movies for an actor
	       aActors.erase (iActor);
	    } // end-if director has actors for movies
	 } // end-for all actors
	 actors.expand_all ();
      } // endif actors available

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 actor", "Loaded %1 actors", actrs.size ())));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (actrs.size ()));
      status.pop ();
      status.push (msg);

      loadedPages |= 4;

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading actors!"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the actors1!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Aktualizes changed entries in the database
//-----------------------------------------------------------------------------
void CDManager::writeChangedEntries () {
   try {
      while (changedInterprets.size ()) {
	 HInterpret artist (changedInterprets.begin ()->first);
	 Check3 (artist.isDefined ());
	 try {
	    std::stringstream query;
	    std::string tmp (escapeDBValue (artist->getName ()));
	    query << (artist->getId () ? "UPDATE Celebrities" : "INSERT INTO Celebrities")
		  << " SET name=\"" << tmp
		  << "\", born="
		  << (artist->getBorn ().isDefined () ? artist->getBorn () : YGP::AYear (0))
		  << ", died="
		  << (artist->getDied ().isDefined () ? artist->getDied () : YGP::AYear (0));

	    if (artist->getId ())
	       query << " WHERE id=" << artist->getId ();
	    Database::execute (query.str ().c_str ());

	    if (!artist->getId ()) {
	       artist->setId (Database::getIDOfInsert ());
	       Database::execute ("INSERT INTO Interprets set id=LAST_INSERT_ID()");
	    }
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write interpret `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, artist->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedInterprets.begin ()->second);
	 changedInterprets.erase (changedInterprets.begin ());
      } // endwhile

      while (changedRecords.size ()) {
	 HRecord record (changedRecords.begin ()->first); Check3 (record.isDefined ());
	 try {
	    std::stringstream query;
	    query << (record->getId () ? "UPDATE Records" : "INSERT INTO Records")
		  << " SET name=\"" << escapeDBValue (record->getName ())
		  << "\", genre=" << record->getGenre ();
	    if (record->getYear ().isDefined ())
	       query << ", year=" << record->getYear ();
	    if (relRecords.isRelated (record)) {
	       HInterpret artist (relRecords.getParent (record)); Check3 (artist.isDefined ());
	       query << ", interpret=" << artist->getId ();
	    }
	    if (record->getId ())
	       query << " WHERE id=" << record->getId ();
	    Database::execute (query.str ().c_str ());

	    if (!record->getId ())
	       record->setId (Database::getIDOfInsert ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write record `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, record->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedRecords.begin ()->second);
	 changedRecords.erase (changedRecords.begin ());
      } // endwhile

      while (changedSongs.size ()) {
	 HSong song (changedSongs.begin ()->first); Check3 (song.isDefined ());
	 try {
	    std::stringstream query;
	    query << (song->getId () ? "UPDATE Songs" : "INSERT INTO Songs")
		  << " SET name=\"" << escapeDBValue (song->getName ())
		  << "\", duration=\"" << song->getDuration () << "\", genre="
		  << song->getGenre ();
	    if (song->getTrack ().isDefined ())
	       query << ", track=" << song->getTrack ();
	    if (relSongs.isRelated (song)) {
	       HRecord record (relSongs.getParent (song)); Check3 (record.isDefined ());
	       query << ", idRecord=" << record->getId ();
	    }
	    if (song->getId ())
	       query << " WHERE id=" << song->getId ();
	    Database::execute (query.str ().c_str ());

	    if (!song->getId ())
	       song->setId (Database::getIDOfInsert ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write song `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, song->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedSongs.begin ()->second);
	 changedSongs.erase (changedSongs.begin ());
      } // endwhile

      while (changedDirectors.size ()) {
	 HDirector director (changedDirectors.begin ()->first);
	 Check3 (director.isDefined ());
	 try {
	    std::stringstream query;
	    query << (director->getId () ? "UPDATE Celebrities" : "INSERT INTO Celebrities")
		  << " SET name=\"" << escapeDBValue (director->getName ())
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
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write director `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, director->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedDirectors.begin ()->second);
	 changedDirectors.erase (changedDirectors.begin ());
      } // endwhile

      while (changedMovies.size ()) {
	 HMovie movie (changedMovies.begin ()->first); Check3 (movie.isDefined ());
	 try {
	    std::stringstream query;
	    query << (movie->getId () ? "UPDATE Movies" : "INSERT INTO Movies")
		  << " SET name=\"" << escapeDBValue (movie->getName (""))
		  << "\", genre=" << movie->getGenre () << ", languages=\""
		  << movie->getLanguage () << "\", subtitles=\""
		  << movie->getTitles () << "\", type=" << movie->getType ();
	    if (movie->getYear ().isDefined ())
	       query << ", year=" << movie->getYear ();
	    if (relMovies.isRelated (movie)) {
	       HDirector director (relMovies.getParent (movie)); Check3 (director.isDefined ());
	       query << ", director=" << director->getId ();
	    }
	    if (movie->getId ())
	       query << " WHERE id=" << movie->getId ();
	    Database::execute (query.str ().c_str ());

	    if (!movie->getId ())
	       movie->setId (Database::getIDOfInsert ());

	    Database::execute ("START TRANSACTION");

	    YGP::Tokenize langs (changedMovieNames[movie]);
	    std::string lang;
	    while ((lang = langs.getNextNode (',')).size ()) {
	       Check3 (Language::exists (lang));
	       std::stringstream del;
	       del << "DELETE FROM MovieNames WHERE id=" << movie->getId ()
		   << " AND language='" << lang << '\'';
	       Database::execute (del.str ().c_str ());

	       if (movie->getName (lang).size ()) {
		  std::stringstream ins;
		  ins << "INSERT INTO MovieNames SET id=" << movie->getId ()
		      << ", name=\"" << escapeDBValue (movie->getName (lang))
		      << "\", language='" << lang << '\'';
		  Database::execute (ins.str ().c_str ());
	       } // endif has translation for language
	    } // endwhile languages changed
	    Database::execute ("COMMIT");
	 }
	 catch (std::exception& err) {
	    Database::execute ("ROLLBACK");

	    Glib::ustring msg (_("Can't write movie `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, movie->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedMovies.begin ()->second);
	 changedMovies.erase (changedMovies.begin ());
      }

      while (changedActors.size ()) {
	 HActor actor (changedActors.begin ()->first);
	 Check3 (actor.isDefined ());
	 try {
	    std::stringstream query;
	    std::string tmp (escapeDBValue (actor->getName ()));
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
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write actor `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, actor->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 Check3 (changedActors.begin ()->second);
	 changedActors.erase (changedActors.begin ());
      } // endwhile

      while (changedRelMovies.size ()) {
	 HActor actor (*changedRelMovies.begin ());
	 Check3 (actor.isDefined ()); Check3 (relActor.isRelated (actor));

	 try {
	    Database::execute ("START TRANSACTION");

	    std::stringstream del;
	    del << "DELETE FROM ActorsInMovies WHERE idActor=" << actor->getId ();
	    Database::execute (del.str ().c_str ());

	    for (std::vector<HMovie>::const_iterator m (relActors.getObjects (actor).begin ());
		 m != relActors.getObjects (actor).end (); ++m) {
	       std::stringstream query;
	       query << "INSERT INTO ActorsInMovies SET idActor=" << actor->getId ()
		     << ", idMovie=" << (*m)->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    Database::execute ("COMMIT");
	 }
	 catch (std::exception& err) {
	    Database::execute ("ROLLBACK");

	    Glib::ustring msg (_("Can't write movies for actor `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, actor->getName ());
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedRelMovies.erase (changedRelMovies.begin ());
      } // end-while
   }
   catch (Glib::ustring& msg) {
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Stores one name into the database
/// \param word: Word to store
//-----------------------------------------------------------------------------
void CDManager::storeWord (const char* word) {
   std::string ins ("INSERT INTO Words VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, word);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Stores one artice into the database
/// \param article: Article to store
//-----------------------------------------------------------------------------
void CDManager::storeArticle (const char* article) {
   std::string ins ("INSERT INTO Articles VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, article);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Edits dthe preferences
//-----------------------------------------------------------------------------
void CDManager::savePreferences () {
   std::ofstream inifile (opt.pINIFile);
   if (inifile) {
      inifile << "[Database]\nUser=" << opt.getUser () << "\nPassword="
	      << opt.getPassword () << "\n\n";

      YGP::INIFile::write (inifile, "Export", opt);

      inifile << "[Movies]\nLanguage=" << Movie::currLang << '\n';
   }
   else {
      Glib::ustring msg (_("Can't create file `%1'!\n\nReason: %2."));
      msg.replace (msg.find ("%1"), 2, opt.pINIFile);
      msg.replace (msg.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }

   // Storing the special/first names and the articles
   try {
      Database::execute ("START TRANSACTION");
      Database::execute ("DELETE FROM Words");
      Words::forEachName (0, Words::cNames (), *this, &CDManager::storeWord);
      Database::execute ("COMMIT");

      Database::execute ("START TRANSACTION");
      Database::execute ("DELETE FROM Articles");
      Words::forEachArticle (0, Words::cArticles (), *this, &CDManager::storeArticle);
      Database::execute ("COMMIT");
   }
   catch (std::exception& e) {
      Glib::ustring msg (_("Can't store special names!\n\nReason: %1."));
      msg.replace (msg.find ("%1"), 2, e.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Removes deleed entries from the database
//-----------------------------------------------------------------------------
void CDManager::removeDeletedEntries () {
   try {
      while (deletedInterprets.size ()) {
	 HInterpret artist (*deletedInterprets.begin ()); Check3 (artist.isDefined ());
	 if (artist->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Interprets WHERE id=" << artist->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete interpret `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, artist->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 deletedInterprets.erase (deletedInterprets.begin ());
      } // endwhile

      while (deletedRecords.size ()) {
	 HRecord record (deletedRecords.begin ()->first); Check3 (record.isDefined ());
	 if (record->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Records WHERE id=" << record->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete record `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, record->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 deletedRecords.erase (deletedRecords.begin ());
      } // endwhile

      while (deletedSongs.size ()) {
	 HSong song (deletedSongs.begin ()->first); Check3 (song.isDefined ());
	 if (song->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Songs WHERE id=" << song->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete song `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, song->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 deletedSongs.erase (deletedSongs.begin ());
      } // end-while

      while (deletedDirectors.size ()) {
	 HDirector director (*deletedDirectors.begin ()); Check3 (director.isDefined ());
	 if (director->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Directors WHERE id=" << director->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete director `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, director->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 deletedDirectors.erase (deletedDirectors.begin ());
      } // endwhile

      while (deletedMovies.size ()) {
	 HMovie movie (deletedMovies.begin ()->first); Check3 (movie.isDefined ());
	 if (movie->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Movies WHERE id=" << movie->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete movie `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, movie->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 deletedMovies.erase (deletedMovies.begin ());
      } // endwhile

      while (deletedActors.size ()) {
	 HActor actor (*deletedActors.begin ()); Check3 (actor.isDefined ());
	 if (actor->getId ()) {
	    try {
	       std::stringstream query;
	       query << "DELETE FROM Actors WHERE id=" << actor->getId ();
	       Database::execute (query.str ().c_str ());
	    }
	    catch (std::exception& err) {
	       Glib::ustring msg (_("Can't delete actor `%1'!\n\nReason: %2"));
	       msg.replace (msg.find ("%1"), 2, actor->getName ());
	       msg.replace (msg.find ("%2"), 2, err.what ());
	       throw (msg);
	    }
	 }
	 relDelActors.unrelateAll (actor);
	 deletedActors.erase (deletedActors.begin ());
      } // endwhile
      Check3 (relDelActors.getParents ().empty ());
   }
   catch (Glib::ustring& msg) {
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Escapes the quotes in values for the database
/// \param value: Value to escape
/// \returns Glib::ustring: Escaped text
//-----------------------------------------------------------------------------
Glib::ustring CDManager::escapeDBValue (const Glib::ustring& value) {
   unsigned int pos (0);
   Glib::ustring rc (value);
   while ((pos = rc.find ('"', pos)) != Glib::ustring::npos) {
      rc.replace (pos, 1, "\\\"");
      pos += 2;
   }
   TRACE9 ("CDManager::escapeDBValue (const Glib::ustring&) - Escaped: " << rc);
   return rc;
}
