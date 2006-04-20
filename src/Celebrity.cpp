//$Id: Celebrity.cpp,v 1.10 2006/04/20 20:35:04 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Celebrity
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.10 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
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
#include "Celebrity.h"
#include "Celebrity.meta"


//-----------------------------------------------------------------------------
/// Copy constructor
/// \param other: Object to clone
//-----------------------------------------------------------------------------
Celebrity::Celebrity (const Celebrity& other)
   : id (other.id), name (other.name), born (other.born), died (other.died) {
}


//-----------------------------------------------------------------------------
/// Assignment operator
/// \param other: Object to assign
/// \returns Celebrity&: Reference to self
//-----------------------------------------------------------------------------
Celebrity& Celebrity::operator= (const Celebrity& other) {
   if (this != &other) {
      if (!id)
	 id = other.id;
      name = other.name;
      born = other.born;
      died = other.died;
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Removes the leading parts (articles, names) of a name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Celebrity::removeIgnored (const Glib::ustring& name) {
   TRACE9 ("Celebrity::removeIgnored (const Glib::ustring&) - " << name);

   Glib::ustring result (name);
   result = Words::removeArticle (name);
   result = Words::removeNames (result);
   return result;
}

//-----------------------------------------------------------------------------
/// Sorts celibritys by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a: First celibrity
/// \param b: Second celibrity
/// \returns bool: True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Celebrity::compByName (const HCelebrity& a, const HCelebrity& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   int rc (removeIgnored (a->name).compare (removeIgnored (b->name)));
   return rc ? (rc < 0) : (a->name < b->name);
}

//-----------------------------------------------------------------------------
/// Sorts celibritys by their id
/// \param a: First celibrity
/// \param b: Second celibrity
/// \returns bool: True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Celebrity::compById (const HCelebrity& a, const HCelebrity& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   return a->getId () < b->getId ();
}

//-----------------------------------------------------------------------------
/// Sets the born and died values from the passed string
/// \param value: Year the celebrity was born/died in format [born][-died]
/// \throw std::exception in case of error
//-----------------------------------------------------------------------------
void Celebrity::setLifespan (const Glib::ustring& value) throw (std::invalid_argument) {
   unsigned int pos (value.find ("- "));
   if (pos != std::string::npos)
      setDied (value.substr (pos + 2));
   else
      died.undefine ();

   if ((pos == std::string::npos)
       || ((pos > 0) && (value[pos - 1] == ' ')))
      setBorn (value.substr (0, pos - 1));
   else
      born.undefine ();
}

//-----------------------------------------------------------------------------
/// Returns the time of live of the passed celebrity
/// \returns Glib::ustring: Text to display
//-----------------------------------------------------------------------------
Glib::ustring Celebrity::getLifespan () const {
   TRACE9 ("Celebrity::getLiveSpan () - " << (isDefined () ? name.c_str () : "None"));
   Check1 (isDefined ());

   Glib::ustring tmp (born.toString ());
   if (died.isDefined ()) {
      if (tmp.size ())
	 tmp.append (1, ' ');
      tmp.append ("- ");
      tmp.append (died.toString ());
   }
   return tmp;
}
