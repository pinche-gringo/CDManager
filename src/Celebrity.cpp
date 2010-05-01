//PROJECT     : CDManager
//SUBSYSTEM   : Celebrity
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2009, 2010

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


#include <cctype>

#include <glibmm/ustring.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ADate.h>

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
   Check1 (a); Check1 (b);
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
   Check1 (a); Check1 (b);
   return a->getId () < b->getId ();
}

//-----------------------------------------------------------------------------
/// Sets the born and died values from the passed string
/// \param value: Year the celebrity was born/died in format [born][-died]
/// \throw std::exception in case of error
//-----------------------------------------------------------------------------
void Celebrity::setLifespan (const Glib::ustring& value) throw (std::invalid_argument) {
   size_t pos (value.find ("- "));
   if ((pos == std::string::npos)
       || ((pos > 0) && (value[pos - 1] == ' '))) {
      YGP::AYear tmp (value.substr (0, pos - 1));
      if (((unsigned int)tmp < 1850U)
	  || (unsigned int)tmp > (unsigned int)YGP::ADate::today ().getYear ())
	 throw std::invalid_argument (_("Invalid birth date!"));

      setBorn (tmp);
   }
   else
      born.undefine ();

   if (pos != std::string::npos) {
      YGP::AYear tmp (value.substr (pos + 2));
      if ((tmp < born) || (unsigned int)tmp > (unsigned int)YGP::ADate::today ().getYear ())
	 throw std::invalid_argument (_("Invalid death date!"));

      setDied (tmp);
   }
   else
      died.undefine ();
}

//-----------------------------------------------------------------------------
/// Returns the time of live of the passed celebrity
/// \returns Glib::ustring: Text to display
//-----------------------------------------------------------------------------
Glib::ustring Celebrity::getLifespan () const {
   TRACE9 ("Celebrity::getLiveSpan () - " << name.c_str ());

   Glib::ustring tmp (born.toString ());
   if (died.isDefined ()) {
      if (tmp.size ())
	 tmp.append (1, ' ');
      tmp.append ("- ");
      tmp.append (died.toString ());
   }
   return tmp;
}
