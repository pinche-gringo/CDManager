//$Id: Interpret.cpp,v 1.1 2004/10/30 17:46:30 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Interpret
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
//COPYRIGHT   : Anticopyright (A) 2004

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

#include "Interpret.h"


std::vector<Glib::ustring> Interpret::ignore;


//-----------------------------------------------------------------------------
/// 
/// \param 
/// \returns 
/// \remarks 
/// \throws 
//-----------------------------------------------------------------------------
Glib::ustring Interpret::removeIgnored (const Glib::ustring& name) {
   TRACE9 ("Interpret::removeIgnored (const Glib::ustring&) - " << name);

   unsigned int pos (0);
   for (std::vector<Glib::ustring>::const_iterator i (ignore.begin ());
	i != ignore.end (); ++i)
      if ((*i == name.substr (pos, i->size ())) && (name[i->size ()] == ' ')) {
	  pos += i->size () + 1;

	  while (isspace (name[pos]))
	     ++pos;
	  i = ignore.begin ();
      }

   TRACE8 ("Interpret::removeIgnored (const Glib::ustring&) - " << name << "->"
	   << name.substr (pos));
   return name.substr (pos);
}

//-----------------------------------------------------------------------------
/// Sorts interprets by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a: First interpret
/// \param b: Second interpret
/// \returns bool: True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Interpret::compByName (const HInterpret& a, const HInterpret& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   Glib::ustring aname (removeIgnored (a->name));
   Glib::ustring bname (removeIgnored (b->name));
   return aname < bname;
}


