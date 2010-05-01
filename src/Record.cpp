//PROJECT     : CDManager
//SUBSYSTEM   : Record
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009, 2010

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


#include <glibmm/ustring.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <XGP/XAttribute.h>  // Needed for specialization of YGP::Attribute for Glib::ustring

#include "Words.h"
#include "Record.h"
#include "Record.meta"


//-----------------------------------------------------------------------------
/// Copyconstructor
/// \param other: Object to copy
//-----------------------------------------------------------------------------
Record::Record (const Record& other)
   : id (other.id), name (other.name),
     year (other.year), genre (other.genre), loadSongs (other.loadSongs) {
 }


//-----------------------------------------------------------------------------
/// Assignment operator
/// \param other: Object to assign
/// \returns Record&: Reference to self
//-----------------------------------------------------------------------------
Record& Record::operator= (const Record& other) {
   if (this != &other) {
      id = other.id;
      name = other.name;
      year = other.year;
      genre = other.genre;
      loadSongs = other.loadSongs;
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Removes the leading articles of the names.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Record::removeIgnored (const Glib::ustring& name) {
   return Words::removeArticle (name);
}

//-----------------------------------------------------------------------------
/// Sorts records by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a: First record
/// \param b: Second record
/// \returns bool: True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Record::compByName (const HRecord& a, const HRecord& b) {
   Check1 (a); Check1 (b);
   Glib::ustring aname (removeIgnored (a->name));
   Glib::ustring bname (removeIgnored (b->name));
   return aname < bname;
}

//-----------------------------------------------------------------------------
/// Sorts records by year.
/// \param a: First record
/// \param b: Second record
/// \returns bool: True, if a->year < b->year
//-----------------------------------------------------------------------------
bool Record::compByYear (const HRecord& a, const HRecord& b) {
   Check1 (a); Check1 (b);
   return a->year < b->year;
}

//-----------------------------------------------------------------------------
/// Sorts records by genre.
/// \param a: First record
/// \param b: Second record
/// \returns bool: True, if a->genre < b->genre
//-----------------------------------------------------------------------------
bool Record::compByGenre (const HRecord& a, const HRecord& b) {
   Check1 (a); Check1 (b);
   return a->genre < b->genre;
}
