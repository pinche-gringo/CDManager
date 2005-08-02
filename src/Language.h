#ifndef LANGUAGE_H
#define LANGUAGE_H

//$Id: Language.h,v 1.6 2005/08/02 01:55:49 markus Rel $

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


#include <map>
#include <string>
#include <stdexcept>

#include <glibmm/ustring.h>

#ifdef USE_LANGUAGEPIXMAPS
#  include <gdkmm/pixbuf.h>
#endif


/**Class to manage languages
 */
struct Language {
 public:
   static void init ();

   static Glib::ustring findInternational (const std::string& lang) throw (std::out_of_range);
   static const Language& findLanguage (const std::string& lang) throw (std::out_of_range);
   static bool exists (const std::string& lang);
#ifdef USE_LANGUAGEPIXMAPS
   static Glib::RefPtr<Gdk::Pixbuf> findFlag (const std::string& lang) throw (std::out_of_range);
#endif

   Glib::ustring getInternational () const { return nameInternational; }
#ifdef USE_LANGUAGEPIXMAPS
   const Glib::RefPtr<Gdk::Pixbuf> getFlag () const {return flag; }
#endif

   Language ();
   Language (const Language& other);
   Language (const Glib::ustring& internat
#ifdef USE_LANGUAGEPIXMAPS
	     , const Glib::RefPtr<Gdk::Pixbuf>& image
#endif
);

   Language& operator= (const Language& other);
   ~Language ();

   static std::map<std::string, Language>::const_iterator begin () {
      return languages.begin (); }
   static std::map<std::string, Language>::const_iterator end () {
      return languages.end (); }

 protected:
#ifdef USE_LANGUAGEPIXMAPS
   static Glib::RefPtr<Gdk::Pixbuf> loadFlag (const char* file);
#endif

 private:
   Glib::ustring nameInternational;
#ifdef USE_LANGUAGEPIXMAPS
   Glib::RefPtr<Gdk::Pixbuf> flag;
#endif

   static std::map<std::string, Language> languages;
};

#endif
