//$Id: Celebrity.cpp,v 1.5 2005/01/10 02:11:44 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Celebrity
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.5 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
//COPYRIGHT   : Copyright (A) 2004, 2005

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
   Glib::ustring aname (removeIgnored (a->name));
   Glib::ustring bname (removeIgnored (b->name));
   return aname < bname;
}
