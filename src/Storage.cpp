//$Id: Storage.cpp,v 1.2 2006/01/24 18:02:17 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Storage
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
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

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "DB.h"
#include "Words.h"

#include "Storage.h"


//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param db: Name of database
/// \param user: User to use for the DB
/// \param pwd: Password of user
/// \throw std::exception: Occurred error
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

   Database::execute ("SELECT article FROM Articles");
   while (Database::hasData ()) {
      // Fill and store artist entry from DB-values
      Words::addArticle (Database::getResultColumnAsString (0), Words::POS_END);
      Database::getNextResultRow ();
   }
}

//-----------------------------------------------------------------------------
/// Stores one name into the database
/// \param word: Word to store
/// \throw std::exception: Occurred error
//-----------------------------------------------------------------------------
void Storage::storeWord (const char* word) throw (std::exception) {
   std::string ins ("INSERT INTO Words VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, word);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Stores one artice into the database
/// \param article: Article to store
/// \throw std::exception: Occurred error
//-----------------------------------------------------------------------------
void Storage::storeArticle (const char* article) throw (std::exception) {
   std::string ins ("INSERT INTO Articles VALUES ('%1')");
   ins.replace (ins.find ("%1"), 2, article);

   Database::execute (ins.c_str ());
}

//-----------------------------------------------------------------------------
/// Deletes all names stored in the database
/// \throw std::exception: Occurred error
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
/// \param target: Vector, in which to load the celebrities
/// \param table: Database table from which to load them
/// \param stat: Statusobject, in which to return the errors
//-----------------------------------------------------------------------------
void Storage::loadCelebrities (std::vector<HCelebrity>& target, const std::string& table,
			       YGP::StatusObject& stat) {
   TRACE9 ("Storage::loadCelebrities (std::vector<HCelebrity>&, const std::string&,\n\tYGP::StatusObject&) - " << table);

   // Load data from Celebrities table
   std::string cmd ("SELECT c.id, c.name, c.born, c.died FROM Celebrities c, ");
   cmd += table;
   cmd += " x WHERE c.id = x.id";
   Database::execute (cmd.c_str ());

   HCelebrity hCeleb;
   while (Database::hasData ()) {
      TRACE5 ("Storage::laodCelebrities () - Adding " << Database::getResultColumnAsUInt (0) << '/' << Database::getResultColumnAsString (1));

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
