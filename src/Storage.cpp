//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 21.01.2006
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

#include "DB.h"
#include "Words.h"

#include "Storage.h"


std::unique_ptr<Database> Storage::database;


//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param db Name of database
/// \param user User to use for the DB
/// \param pwd Password of user
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::login (const char* db, const char* user, const char* pwd) throw (std::exception) {
   std::unique_ptr<Database> newDB (Database::create ());
   newDB->connect (db, user, pwd);
   database = std::move (newDB);
}

//-----------------------------------------------------------------------------
/// Log-out from the database
//-----------------------------------------------------------------------------
void Storage::logout () {
   if (database) {
      database->close ();
      database.reset ();
   }
}

//-----------------------------------------------------------------------------
/// Checks if the connection to the database is established
/// \returns bool: True, if connections is established
//-----------------------------------------------------------------------------
bool Storage::connected () {
   return database && database->connected ();
}

//-----------------------------------------------------------------------------
/// Returns the database to work with
/// \returns Database& The database
/// \throw std::exception If not logged in
//-----------------------------------------------------------------------------
Database& Storage::db () {
   if (!database)
      throw std::runtime_error ("Not connected to the database");
   return *database;
}

//-----------------------------------------------------------------------------
/// Loads the special words from the database
//-----------------------------------------------------------------------------
void Storage::loadSpecialWords () throw (std::exception) {
   Words::create ();

   db ().execute ("SELECT word FROM Words");
   while (db ().hasData ()) {
      // Fill and store artist entry from DB-values
      Words::addName2Ignore (db ().getResultColumnAsString (0), Words::POS_END);
      db ().getNextResultRow ();
   }
   TRACE1 ("Storage::loadSpecialWords () - " << Words::cNames() << '/' << Words::cArticles());

   db ().execute ("SELECT article FROM Articles");
   while (db ().hasData ()) {
      // Fill and store artist entry from DB-values
      Words::addArticle (db ().getResultColumnAsString (0), Words::POS_END);
      db ().getNextResultRow ();
   }
   TRACE1 ("Storage::loadSpecialWords () - " << Words::cNames() << '/' << Words::cArticles());
}

//-----------------------------------------------------------------------------
/// Stores one name into the database
/// \param word Word to store
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::storeWord (const char* word) throw (std::exception) {
   db ().execute ("INSERT INTO Words VALUES (" + db ().quote (word) + ')');
}

//-----------------------------------------------------------------------------
/// Stores one artice into the database
/// \param article Article to store
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::storeArticle (const char* article) throw (std::exception) {
   db ().execute ("INSERT INTO Articles VALUES (" + db ().quote (article) + ')');
}

//-----------------------------------------------------------------------------
/// Deletes all names stored in the database
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::deleteNames () throw (std::exception) {
   db ().execute ("DELETE FROM Words");
}

//-----------------------------------------------------------------------------
/// Deletes all articles stored in the database
//-----------------------------------------------------------------------------
void Storage::deleteArticles () throw (std::exception) {
   db ().execute ("DELETE FROM Articles");
}

//-----------------------------------------------------------------------------
/// Loads the stored celebrities from the database
/// \param target Vector, in which to load the celebrities
/// \param table Database table from which to load them
/// \param stat Statusobject, in which to return the errors
//-----------------------------------------------------------------------------
void Storage::loadCelebrities (std::vector<HCelebrity>& target, const std::string& table,
			       YGP::StatusObject& stat) throw (std::exception) {
   TRACE9 ("Storage::loadCelebrities (std::vector<HCelebrity>&, const std::string&,\n\tYGP::StatusObject&) - " << table);

   // Load data from Celebrities table
   std::string cmd ("SELECT c.id, c.name, c.born, c.died FROM Celebrities c, ");
   cmd += table;
   cmd += " x WHERE c.id = x.id";
   db ().execute (cmd.c_str ());
   fillCelebrities (target, stat);
}

//-----------------------------------------------------------------------------
/// Fills the celebrities into the passed vector
/// \param target Vector to fill with celebrities
/// \param stat Object to hold status-information
//-----------------------------------------------------------------------------
void Storage::fillCelebrities (std::vector<HCelebrity>& target, YGP::StatusObject& stat) {
   HCelebrity hCeleb;
   while (db ().hasData ()) {
      TRACE5 ("Storage::fillCelebrities (std::vector<HCelebrity>&, YGP::StatusObject&)) - Adding " << db ().getResultColumnAsUInt (0) << '/' << db ().getResultColumnAsString (1));

      // Fill and store entry from DB-values
      try {
	 hCeleb.reset (new Celebrity);
	 hCeleb->setId (db ().getResultColumnAsUInt (0));
	 hCeleb->setName (db ().getResultColumnAsString (1));

	 unsigned int tmp (db ().getResultColumnAsUInt (2));
	 if (tmp != 0)
	    hCeleb->setBorn (tmp);
	 tmp = db ().getResultColumnAsUInt (3);
	 if (tmp != 0)
	    hCeleb->setDied (tmp);
      }
      catch (std::exception& e) {
	 Glib::ustring msg (_("Warning loading celebrity `%1': %2"));
	 msg.replace (msg.find ("%1"), 2, hCeleb->getName ());
	 msg.replace (msg.find ("%2"), 2, e.what ());
	 stat.setMessage (YGP::StatusObject::WARNING, msg);
      }
      target.push_back (hCeleb);

      db ().getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Starts a database-transaction
//-----------------------------------------------------------------------------
void Storage::startTransaction () {
   db ().execute ("START TRANSACTION");
}

//-----------------------------------------------------------------------------
/// Aborts a database-transaction
//-----------------------------------------------------------------------------
void Storage::abortTransaction () {
   db ().execute ("ROLLBACK");
}

//-----------------------------------------------------------------------------
/// Commits a database-transaction
//-----------------------------------------------------------------------------
void Storage::commitTransaction () {
   db ().execute ("COMMIT");
}

//-----------------------------------------------------------------------------
/// Returns the columns of the passed celebrity to store in the database
/// \param celeb Celebrity to store
/// \returns Database::Values Columns and their values
//-----------------------------------------------------------------------------
Database::Values Storage::celebrityValues (const HCelebrity celeb) {
   Database::Values values;
   values ("name", db ().quote (celeb->getName ()))
      ("born", celeb->getBorn ().isDefined () ? celeb->getBorn () : YGP::AYear (0))
      ("died", celeb->getDied ().isDefined () ? celeb->getDied () : YGP::AYear (0));
   return values;
}

//-----------------------------------------------------------------------------
/// Saves the passed interpret.
/// \param interpret Interpret to save
/// \returns bool True, if entry was created, false if updated
/// \throw std::exception In case of error
//-----------------------------------------------------------------------------
void Storage::insertCelebrity (const HCelebrity celeb, const char* role) throw (std::exception) {
   Check1 (celeb);
   TRACE8 ("Storage::insertCelebrity (const HCelebrity, const char*) - " << role << ": " << celeb->getName ());
   Check1 (!celeb->getId ());

   db ().insert ("Celebrities", celebrityValues (celeb));
   celeb->setId (db ().getIDOfInsert ());
   setRole (celeb->getId (), role);
}

//-----------------------------------------------------------------------------
/// Updates the passed interpret.
/// \param interpret Interpret to save
/// \returns bool True, if entry was created, false if updated
/// \throw std::exception In case of error
//-----------------------------------------------------------------------------
void Storage::updateCelebrity (const HCelebrity celeb) throw (std::exception) {
   Check1 (celeb);
   TRACE8 ("Storage::updateCelebrity (const HCelebrity) - " << celeb->getName ());
   Check1 (celeb->getId ());

   std::stringstream where;
   where << "id=" << celeb->getId ();
   db ().update ("Celebrities", celebrityValues (celeb), where.str ());
}

//-----------------------------------------------------------------------------
/// Gets the celebrities with the passed name
/// \param name Name of celebrity to query
/// \param target Vector to store the found celebrities
/// \returns unsigned long Id of found celebrity or 0, if not found
/// \throw std::exception In case of error
//-----------------------------------------------------------------------------
void Storage::getCelebrities (const std::string& name, std::vector<HCelebrity>& target) throw (std::exception) {
   YGP::StatusObject stat;
   std::stringstream query;
   query << "SELECT id, name, born, died FROM Celebrities WHERE name=" << db ().quote (name);
   db ().execute (query.str ());
   fillCelebrities (target, stat);
}

//-----------------------------------------------------------------------------
/// Checks if the passed celebrity has a certain role
/// \param idCeleb ID of celebrity
/// \param role Role of celebrity
/// \returns bool True, if the celebrity has the passed role
/// \throw std::exception In case of an error
/// \remarks The roles are the name of the DB-tables
//-----------------------------------------------------------------------------
bool Storage::hasRole (unsigned int idCeleb, const char* role) throw (std::exception) {
   std::stringstream query;
   query << "SELECT id FROM " << role << " WHERE id=" << idCeleb;
   db ().execute (query.str ());
   return db ().hasData ();
}

//-----------------------------------------------------------------------------
/// Sets a role for a celebrity
/// \param idCeleb ID of celebrity
/// \param role Role to set for celebrity
/// \throw std::exception In case of an error
/// \remarks The roles are the name of the DB-tables
//-----------------------------------------------------------------------------
void Storage::setRole (unsigned int idCeleb, const char* role) throw (std::exception) {
   std::stringstream query;
   query << "INSERT INTO " << role << " (id) VALUES (" << idCeleb << ')';
   db ().execute (query.str ());
}

//-----------------------------------------------------------------------------
/// Queries the number of entries in the database
/// \param counts Array receiving the statistical information in order
///               words/articles/interpret/records/director/films/actors
/// \param role: Role to set for celebrity
/// \throw std::exception In case of error
/// \remarks If some pages are disabled the responding columns are returned as -1
//-----------------------------------------------------------------------------
void Storage::getStatistics (int counts[7]) throw (std::exception) {
   const char* query ("SELECT count(*) FROM Words UNION ALL SELECT count(*) FROM Articles UNION ALL "
#ifdef WITH_RECORDS
		      "SELECT count(*) FROM Interprets UNION ALL SELECT count(*) FROM Records"
#else
		      "SELECT -1 UNION ALL SELECT -1"
#endif
		      " UNION ALL "
#ifdef WITH_FILMS
		      "SELECT count(*) FROM Directors UNION ALL SELECT count(*) FROM Films"
#else
		      "SELECT -1 UNION ALL SELECT -1"
#endif
		      " UNION ALL "
#ifdef WITH_ACTORS
		      "SELECT count(*) FROM Actors"
#else
		      "SELECT -1"
#endif
		      );
   db ().execute (query);
   while (db ().hasData ()) {
      *counts++ = db ().getResultColumnAsInt (0);
      db ().getNextResultRow ();
   }
}
