//$Id: Language.cpp,v 1.2 2004/12/09 03:19:05 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
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


#include "cdmgr-cfg.h"

#include <glibmm/convert.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Language.h"


std::map<std::string, Language> Language::languages;


//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
Language::Language () {
 }

//-----------------------------------------------------------------------------
/// Copyconstructor
/// \param other: Object to copy
//-----------------------------------------------------------------------------
Language::Language (const Language& other)
   : nameNational (other.nameNational), nameInternational (other.nameInternational),
     flag (other.flag) {
 }

//-----------------------------------------------------------------------------
/// Constructor from values
/// \param internat: Name of the language (in english)
/// \param national: Name of the language (in its language)
/// \param image: Image describing the language (flag)
//-----------------------------------------------------------------------------
Language::Language (const Glib::ustring& internat, const Glib::ustring& national,
		    const Glib::RefPtr<Gdk::Pixbuf>& image)
   : nameNational (national), nameInternational (internat), flag (image) {
   Check2 (flag);
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
      nameNational = other.nameNational;
      nameInternational = other.nameInternational;
      flag = other.flag;
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Initializes the articles
//-----------------------------------------------------------------------------
void Language::init () {
   languages["de"] = Language (N_("German"), Glib::locale_to_utf8 ("Deutsch"),
			       loadFlag (DATADIR "de.png"));
   languages["en"] = Language (N_("English"), Glib::locale_to_utf8 ("English"),
			       loadFlag (DATADIR "en.png"));
   languages["es"] = Language (N_("Spanish"), Glib::locale_to_utf8 ("Español"),
			       loadFlag (DATADIR "es.png"));
   languages["fr"] = Language (N_("French"), Glib::locale_to_utf8 ("Frances"),
			       loadFlag (DATADIR "fr.png"));
   languages["it"] = Language (N_("Italian"), Glib::locale_to_utf8 ("Italiano"),
			       loadFlag (DATADIR "it.png"));
   languages["pt"] = Language (N_("Portugese"), Glib::locale_to_utf8 ("Português"),
			       loadFlag (DATADIR "pt.png"));
}

//-----------------------------------------------------------------------------
/// Returns the name (in itself) of the passed language (e.g. "Deutsch")
/// \param lang: Language to get
/// \returns Glib::ustring: Language
/// \throws std::out_of_range: If the value does not exist
//-----------------------------------------------------------------------------
Glib::ustring Language::findNational (const std::string& lang) throw (std::out_of_range) {
   std::map<std::string, Language>::const_iterator i (languages.find (lang));
   if (i != languages.end ())
      return i->second.nameNational;
   throw std::out_of_range ("Language::findNational (const std::string&)");
}

//-----------------------------------------------------------------------------
/// Returns the name (in english) of the passed language (e.g. "Spanish")
/// \param lang: Language to get
/// \returns Glib::ustring: Language
/// \throws std::out_of_range: If the value does not exist
//-----------------------------------------------------------------------------
Glib::ustring Language::findInternational (const std::string& lang) throw (std::out_of_range) {
   std::map<std::string, Language>::const_iterator i (languages.find (lang));
   if (i != languages.end ())
      return i->second.nameInternational;
   throw std::out_of_range ("Language::findInternational (const std::string&)");
}

//-----------------------------------------------------------------------------
/// Returns the name (in english) of the passed language (e.g. "Spanish")
/// \param lang: Language to get
/// \returns Glib::ustring: Language
/// \throws std::out_of_range: If the value does not exist
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::Pixbuf> Language::findFlag (const std::string& lang) throw (std::out_of_range) {
   std::map<std::string, Language>::const_iterator i (languages.find (lang));
   if (i != languages.end ())
      return i->second.flag;
   throw std::out_of_range ("Language::findFlag (const std::string&)");
}

//-----------------------------------------------------------------------------
/// Checks if the passed string-value exists within the languages
/// \param value: Value to check for
/// \returns bool
/// \remarks value must exist within the enum
//-----------------------------------------------------------------------------
bool Language::exists (const std::string& lang) {
   if (lang.size () == 2) {
      std::map<std::string, Language>::const_iterator i (languages.find (lang));
      if (i != languages.end ())
	 return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Loads the flag image
/// \param path: Path to image file
/// \returns Glib::RefPtr<Gdk::Pixbuf>: Pointer to created image
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::Pixbuf> Language::loadFlag (const char* file) {
   TRACE9 ("Language::loadFlag (const char*) - " <<  file);
   Check1 (file);
   try {
      return Gdk::Pixbuf::create_from_file (file);
   }
   catch (Gdk::PixbufError& e) {
      TRACE1 ("Language::loadFlag (const char*) - " <<  e.what ());
   }
   catch (Glib::FileError& e) {
      TRACE1 ("Language::loadFlag (const char*) - " <<  e.what ());
   }
   return Glib::RefPtr<Gdk::Pixbuf> ();
}
