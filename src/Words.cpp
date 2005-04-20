//$Id: Words.cpp,v 1.7 2005/04/20 05:43:09 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Words
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.7 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
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


#include <cdmgr-cfg.h>

#include <cctype>
#include <algorithm>

#include <glibmm/ustring.h>

#include <YGP/Trace.h>
#include <YGP/Check.h>

#include "Words.h"


std::vector<Glib::ustring> Words::names;
std::vector<Glib::ustring> Words::articles;



//-----------------------------------------------------------------------------
/// Sorts the articles
//-----------------------------------------------------------------------------
void Words::sortArticles () {
   std::sort (articles.begin (), articles.end (), &Words::compare);
}

//-----------------------------------------------------------------------------
/// Sorts the names
//-----------------------------------------------------------------------------
void Words::sortNames () {
   std::sort (names.begin (), names.end (), &Words::compare);
}

//-----------------------------------------------------------------------------
/// Compares two words
/// \param w1: First word to compare
/// \param w2: Second word to compare
/// \returns bool: True, if w1 < w2
//-----------------------------------------------------------------------------
bool Words::compare (const Glib::ustring& w1, const Glib::ustring& w2) {
   return w1 < w2;
}

//-----------------------------------------------------------------------------
/// Returns the first word of the passed string
/// \param text: Text to extract the first word from
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Words::getWord (const Glib::ustring& text) {
   unsigned int i (-1U);
   while (++i < text.size ())
      if (isspace (text[i]) || (text[i] == '-'))
	  break;

   TRACE9 ("Words::getWord (const Glib::ustring&) - '" << text.substr (0, i) << '\'');
   return text.substr (0, i);
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article or name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeArticle (const Glib::ustring& name) {
   TRACE9 ("Words::removeArticles (const Glib::ustring&) - " << name);

   Glib::ustring word (getWord (name));
   if (word.size () != name.size ()
       && containsWord (articles, word)) {
      unsigned int pos (word.size ());
      while (!isalnum (name[pos]))
	 ++pos;

      TRACE3 ("Words::removeArticles (const Glib::ustring&) - " << name << "->"
	      << name.substr (pos));
      return name.substr (pos);
   }
   return name;
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article or name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeNames (const Glib::ustring& name) {
   TRACE9 ("Words::removeNames (const Glib::ustring&) - " << name);

   Glib::ustring work (name);
   Glib::ustring word (getWord (work));
   while ((word.size () != name.size ()) && containsWord (names, word)) {
      unsigned int pos (word.size ());
      while (!isalnum (name[pos]))
	 ++pos;

      work = work.substr (pos);
      word = getWord (work);
   }
   TRACE3 ("Words::removeName (const Glib::ustring&) - " << name << "->" << work);
   return work;
}

//-----------------------------------------------------------------------------
/// Checks if the passed list contains the passed word
/// \param list: List to inspect
/// \param word: Word to search for
/// \returns bool: True, if the word exists
//-----------------------------------------------------------------------------
bool Words::containsWord (const std::vector<Glib::ustring>& list,
			  const Glib::ustring& word) {
   std::vector<Glib::ustring>::const_iterator i
      (upper_bound (list.begin (), list.end (), word));
   return ((i != list.begin ()) && (word == *--i));
}
