//$Id: Language.cpp,v 1.1 2004/12/07 03:33:57 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 6.12.2004
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


#include <glibmm/convert.h>

#include "Language.h"


std::map<LANG, Language> Language::languages;


//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
Language::Language () : id ("??") {
 }

//-----------------------------------------------------------------------------
/// Copyconstructor
/// \param other: Object to copy
//-----------------------------------------------------------------------------
Language::Language (const Language& other)
   : id (other.id), nameNational (other.nameNational), nameInternational (other.nameInternational) {
 }

//-----------------------------------------------------------------------------
/// Constructor from values
/// \param id: Short value for language (e.g. "en", "de", ...)
/// \param internat: Name of the language (in english) 
/// \param national: Name of the language (in its language) 
//-----------------------------------------------------------------------------
Language::Language (const char* id, const Glib::ustring& internat,
		    const Glib::ustring& national)
   : id (id), nameNational (national), nameInternational (internat) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Language::~Language () {
}


//-----------------------------------------------------------------------------
/// Assignment operator
/// \param other: Object to clone
/// \returns Language&: Self
//-----------------------------------------------------------------------------
Language& Language::operator= (const Language& other) {
   if (this != &other) {
      id = other.id;
      nameNational = other.nameNational;
      nameInternational = other.nameInternational;
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Initializes the articles
//-----------------------------------------------------------------------------
void Language::init () {
   languages[ENGLISH] = Language ("en", Glib::locale_to_utf8 ("English"), Glib::locale_to_utf8 ("English"));
   languages[GERMAN] = Language ("de", Glib::locale_to_utf8 ("German"), Glib::locale_to_utf8 ("Deutsch"));
   languages[FRENCH] = Language ("fr", Glib::locale_to_utf8 ("French"), Glib::locale_to_utf8 ("Frances"));
   languages[ITALIAN] = Language ("it", Glib::locale_to_utf8 ("Italian"), Glib::locale_to_utf8 ("Italiano"));
   languages[SPANISH] = Language ("es", Glib::locale_to_utf8 ("Spanish"), Glib::locale_to_utf8 ("Español"));
}
