//$Id: Movie.cpp,v 1.5 2004/12/13 02:33:00 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Movie
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.5 $
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (A) 2004

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

#include "Words.h"
#include "Movie.h"


//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
Movie::Movie () : id (0), genre (0), type (0) {
}

//-----------------------------------------------------------------------------
/// Copyonstructor
//-----------------------------------------------------------------------------
Movie::Movie (const Movie& other)
   : id (other.id), name (other.name), year (other.year),
     genre (other.genre), type (other.type), lang (other.lang),
     titles (other.titles) {
 }


//-----------------------------------------------------------------------------
/// Removes the leading articles of the names.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Movie::removeIgnored (const Glib::ustring& name) {
   return Words::removeArticle (name);
}

//-----------------------------------------------------------------------------
/// Sorts movies by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a: First movie
/// \param b: Second movie
/// \returns bool: True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Movie::compByName (const HMovie& a, const HMovie& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   Glib::ustring aname (removeIgnored (a->name));
   Glib::ustring bname (removeIgnored (b->name));
   return aname < bname;
}

//-----------------------------------------------------------------------------
/// Sorts movies by year.
/// \param a: First movie
/// \param b: Second movie
/// \returns bool: True, if a->year < b->year
//-----------------------------------------------------------------------------
bool Movie::compByYear (const HMovie& a, const HMovie& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->year < b->year;
}

//-----------------------------------------------------------------------------
/// Sorts movies by genre.
/// \param a: First movie
/// \param b: Second movie
/// \returns bool: True, if a->genre < b->genre
//-----------------------------------------------------------------------------
bool Movie::compByGenre (const HMovie& a, const HMovie& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->genre < b->genre;
}

//-----------------------------------------------------------------------------
/// Sorts movies by media.
/// \param a: First movie
/// \param b: Second movie
/// \returns bool: True, if a->type < b->type
//-----------------------------------------------------------------------------
bool Movie::compByMedia (const HMovie& a, const HMovie& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->type < b->type;
}
