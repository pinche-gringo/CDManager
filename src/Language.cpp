//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 6.12.2004
//COPYRIGHT   : Copyright (C) 2004, 2005, 2010, 2012

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


#include <cdmgr-cfg.h>

#include <glibmm/convert.h>
#include <glibmm/fileutils.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Language.h"

#ifdef USE_LANGUAGEPIXMAPS
#  define LOADFLAG(lang)    , loadFlag (DATADIR #lang ".png")
#else
#  define LOADFLAG(lang)
#endif


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
   : nameInternational (other.nameInternational)
#ifdef USE_LANGUAGEPIXMAPS
     , flag (other.flag)
#endif
 {
 }

//-----------------------------------------------------------------------------
/// Constructor from values
/// \param internat: Name of the language (in english)
/// \param image: Image describing the language (flag)
//-----------------------------------------------------------------------------
Language::Language (const Glib::ustring& internat
#ifdef USE_LANGUAGEPIXMAPS
		    , const Glib::RefPtr<Gdk::Pixbuf>& image
#endif
)
   : nameInternational (internat)
#ifdef USE_LANGUAGEPIXMAPS
     , flag (image)
#endif
 {
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
      nameInternational = other.nameInternational;
#ifdef USE_LANGUAGEPIXMAPS
      flag = other.flag;
#endif
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Initializes the articles
//-----------------------------------------------------------------------------
void Language::init () {
   languages.insert (languages.end (), langValue ("de", Language (_("German") LOADFLAG(de))));
   languages.insert (languages.end (), langValue ("en", Language (_("English") LOADFLAG(en))));
   languages.insert (languages.end (), langValue ("es", Language (_("Spanish") LOADFLAG(es))));
   languages.insert (languages.end (), langValue ("fi", Language (_("Finnish") LOADFLAG(fi))));
   languages.insert (languages.end (), langValue ("fr", Language (_("French") LOADFLAG(fr))));
   languages.insert (languages.end (), langValue ("it", Language (_("Italian") LOADFLAG(it))));
   languages.insert (languages.end (), langValue ("no", Language (_("Norwegian") LOADFLAG(no))));
   languages.insert (languages.end (), langValue ("pl", Language (_("Polish") LOADFLAG(pl))));
   languages.insert (languages.end (), langValue ("pt", Language (_("Portugese") LOADFLAG(pt))));
   languages.insert (languages.end (), langValue ("rs", Language (_("Serbian") LOADFLAG(rs))));
   languages.insert (languages.end (), langValue ("sv", Language (_("Swedish") LOADFLAG(sv))));
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

#ifdef USE_LANGUAGEPIXMAPS
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
#endif

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

#ifdef USE_LANGUAGEPIXMAPS
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
#endif

