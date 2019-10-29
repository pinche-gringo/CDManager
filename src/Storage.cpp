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


//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param db Name of database
/// \param user User to use for the DB
/// \param pwd Password of user
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::login (const char* db, const char* user, const char* pwd) throw (std::exception) {
   Database::connect (db, user, pwd);
}

//-----------------------------------------------------------------------------
/// Log-out from the database
//-----------------------------------------------------------------------------
void Storage::logout () {
   Database::close ();
}

//-----------------------------------------------------------------------------
/// Checks if the connection to the database is established
/// \returns bool: True, if connections is established
//-----------------------------------------------------------------------------
bool Storage::connected () {
   return Database::connected ();
}

//-----------------------------------------------------------------------------
/// Loads the special words from the database
//-----------------------------------------------------------------------------
void Storage::loadSpecialWords () throw (std::exception) {
   Words::create ();

   Database::execute ("SELECT word FROM Words");
   while (Database::hasData ()) {
      // Fill and store artist entry from DB-values
      Words::addName2Ignore (Database::getResultColumnAsString (0), Words::POS_END);
      Database::getNextResultRow ();
   }
   TRACE1 ("Storage::loadSpecialWords () - " << Words::cNames() << '/' << Words::cArticles());

   Database::execute ("SELECT article FROM Articles");
   while (Database::hasData ()) {
      // Fill and store artist entry from DB-values
      Words::addArticle (Database::getResultColumnAsString (0), Words::POS_END);
      Database::getNextResultRow ();
   }
   TRACE1 ("Storage::loadSpecialWords () - " << Words::cNames() << '/' << Words::cArticles());
}

//-----------------------------------------------------------------------------
/// Stores one name into the database
/// \param word Word to store
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::storeWord (const char* word) throw (std::exception) {
   std::string ins ("INSERT INTO Words VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, word);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Stores one artice into the database
/// \param article Article to store
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::storeArticle (const char* article) throw (std::exception) {
   std::string ins ("INSERT INTO Articles VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, article);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes all names stored in the database
/// \throw std::exception Occurred error
//-----------------------------------------------------------------------------
void Storage::deleteNames () throw (std::exception) {
   Database::execute ("DELETE FROM Words");
}

//-----------------------------------------------------------------------------
/// Deletes all articles stored in the database
//-----------------------------------------------------------------------------
void Storage::deleteArticles () throw (std::exception) {
   Database::execute ("DELETE FROM Articles");
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
   Database::execute (cmd.c_str ());
   fillCelebrities (target, stat);
}

//-----------------------------------------------------------------------------
/// Fills the celebrities into the passed vector
/// \param target Vector to fill with celebrities
/// \param stat Object to hold status-information
//-----------------------------------------------------------------------------
void Storage::fillCelebrities (std::vector<HCelebrity>& target, YGP::StatusObject& stat) {
   HCelebrity hCeleb;
   while (Database::hasData ()) {
      TRACE5 ("Storage::fillCelebrities (std::vector<HCelebrity>&, YGP::StatusObject&)) - Adding " << Database::getResultColumnAsUInt (0) << '/' << Database::getResultColumnAsString (1));

      // Fill and store entry from DB-values
      try {
	 hCeleb.reset (new Celebrity);
	 hCeleb->setId (Database::getResultColumnAsUInt (0));
	 hCeleb->setName (Database::getResultColumnAsString (1));

	 unsigned int tmp (Database::getResultColumnAsUInt (2));
	 if (tmp != 0)
	    hCeleb->setBorn (tmp);
	 tmp = Database::getResultColumnAsUInt (3);
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

      Database::getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Starts a database-transaction
//-----------------------------------------------------------------------------
void Storage::startTransaction () {
   Database::execute ("START TRANSACTION");
}

//-----------------------------------------------------------------------------
/// Aborts a database-transaction
//-----------------------------------------------------------------------------
void Storage::abortTransaction () {
   Database::execute ("ROLLBACK");
}

//-----------------------------------------------------------------------------
/// Commits a database-transaction
//-----------------------------------------------------------------------------
void Storage::commitTransaction () {
   Database::execute ("COMMIT");
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

   std::stringstream query;
   query << "INSERT INTO Celebrities  SET name=\"" << Database::escapeDBValue (celeb->getName ())
	 << "\", born="
	 << (celeb->getBorn ().isDefined () ? celeb->getBorn () : YGP::AYear (0))
	 << ", died="
	 << (celeb->getDied ().isDefined () ? celeb->getDied () : YGP::AYear (0));
   Database::execute (query.str ().c_str ());
   celeb->setId (Database::getIDOfInsert ());
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

   std::stringstream query;
   query << "UPDATE Celebrities SET name=\"" << Database::escapeDBValue (celeb->getName ())
	 << "\", born="
	 << (celeb->getBorn ().isDefined () ? celeb->getBorn () : YGP::AYear (0))
	 << ", died="
	 << (celeb->getDied ().isDefined () ? celeb->getDied () : YGP::AYear (0))
	 << " WHERE id=" << celeb->getId ();
   Database::execute (query.str ().c_str ());
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
   query << "SELECT id, name, born, died FROM Celebrities WHERE name=\"" << Database::escapeDBValue (name) << '"';
   Database::execute (query.str ().c_str ());
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
   Database::execute (query.str ().c_str ());
   return Database::hasData ();
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
   query << "INSERT INTO " << role << " set id="<< idCeleb;
   Database::execute (query.str ().c_str ());
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
   Database::execute (query);
   while (Database::hasData ()) {
      *counts++ = Database::getResultColumnAsInt (0);
      Database::getNextResultRow ();
   }
}
