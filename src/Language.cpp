//$Id: Language.cpp,v 1.6 2005/01/31 05:15:01 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.6 $
//AUTHOR      : Markus Schwab
//CREATED     : 6.12.2004
//COPYRIGHT   : Copyright (C) 2004, 2005

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
/// \param loadFlags: Flag if the flags should be loaded
//-----------------------------------------------------------------------------
void Language::init (bool loadFlags) {
   Glib::RefPtr<Gdk::Pixbuf> null;
   languages["de"] = Language (_("German"), Glib::locale_to_utf8 ("Deutsch"),
			       loadFlags ? loadFlag (DATADIR "de.png") : null);
   languages["en"] = Language (_("English"), Glib::locale_to_utf8 ("English"),
			       loadFlags ? loadFlag (DATADIR "en.png") : null);
   languages["es"] = Language (_("Spanish"), Glib::locale_to_utf8 ("Español"),
			       loadFlags ? loadFlag (DATADIR "es.png") : null);
   languages["fr"] = Language (_("French"), Glib::locale_to_utf8 ("Français"),
			       loadFlags ? loadFlag (DATADIR "fr.png") : null);
   languages["it"] = Language (_("Italian"), Glib::locale_to_utf8 ("Italiano"),
			       loadFlags ? loadFlag (DATADIR "it.png") : null);
   languages["pt"] = Language (_("Portugese"), Glib::locale_to_utf8 ("Português"),
			       loadFlags ? loadFlag (DATADIR "pt.png") : null);
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
/// Returns the whole Language-entity
/// \param lang: Language to get
/// \returns const Language&: Language
/// \throws std::out_of_range: If the value does not exist
//-----------------------------------------------------------------------------
const Language& Language::findLanguage (const std::string& lang) throw (std::out_of_range) {
   std::map<std::string, Language>::const_iterator i (languages.find (lang));
   if (i != languages.end ())
      return i->second;
   throw std::out_of_range ("Language::findLanguage (const std::string&)");
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
