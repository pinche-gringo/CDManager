#ifndef LANGUAGE_H
#define LANGUAGE_H

//$Id: Language.h,v 1.1 2004/12/07 03:33:57 markus Exp $

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

#include <glibmm/ustring.h>


/**Class to manage languages
 */
struct Language {
 public:
   typedef enum { ENGLISH = 0, FRENCH, GERMAN, ITALIAN, SPANISH } LANG;
   static void init ();

   Language ();
   Language (const Language& other);
   Language (const char* id, const Glib::ustring& internatnat,
	     const Glib::ustring& national);
   ~Language ();

   Language& operator= (const Language& other);

 private:
   const char*   id;
   Glib::ustring nameNational;
   Glib::ustring nameInternational;

   static std::map<LANG, Language> languages;
};

typedef Language::LANG LANG;

#endif
