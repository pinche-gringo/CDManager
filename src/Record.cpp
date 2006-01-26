//$Id: Record.cpp,v 1.7 2006/01/26 23:09:50 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Record
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.7 $
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006

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


#include <cctype>

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
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
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
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->year < b->year;
}

//-----------------------------------------------------------------------------
/// Sorts records by genre.
/// \param a: First record
/// \param b: Second record
/// \returns bool: True, if a->genre < b->genre
//-----------------------------------------------------------------------------
bool Record::compByGenre (const HRecord& a, const HRecord& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->genre < b->genre;
}
