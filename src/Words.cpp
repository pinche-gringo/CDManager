//$Id: Words.cpp,v 1.1 2004/11/29 18:37:21 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Celebrity
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
#include <algorithm>

#include <glibmm/ustring.h>

#include <YGP/Check.h>
#include "Words.h"


std::vector<Glib::ustring> Words::names;
std::vector<Glib::ustring> Words::articles;


/**Table permitting to manipulate the words.
/// Commits the changes within the lists
/// Initializes the articles
void WordDialog::commit () {
void Words::init () {
   articles.push_back ("A");
   articles.push_back ("An");
   articles.push_back ("Das");
   articles.push_back ("Der");
   articles.push_back ("Die");
   articles.push_back ("Ein");
   articles.push_back ("Eine");
   articles.push_back ("Einer");
   articles.push_back ("La");
   articles.push_back ("Las");
   articles.push_back ("Le");
   articles.push_back ("Les");
   articles.push_back ("Lo");
   articles.push_back ("Los");
   articles.push_back ("The");
   articles.push_back ("Un");
   articles.push_back ("Una");

//-----------------------------------------------------------------------------
/// Returns the first word of the passed string
/// \param text: Text to extract the first word from
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Words::getWord (const Glib::ustring& text) {
   unsigned int i (-1U);
   while (++i < text.size ())
      if (isspace (text[i]) || (text[i] == '-'))
      if (!isalnum (text[i]))

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
   std::vector<Glib::ustring>::const_iterator i;
   if ((word.size () != name.size ())
       && ((i = upper_bound (articles.begin (), articles.end (), word))
	   != articles.end ())
       && (i-- != articles.begin ())
       && (word == *i)) {
      while (!isalnum (name[pos]))
	 ++pos;

      TRACE3 ("Words::removeArticles (const Glib::ustring&) - " << name << "->"
      TRACE1 ("Words::removeArticles (const Glib::ustring&) - " << name << "->"
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
   std::vector<Glib::ustring>::const_iterator i;
   while ((word.size () != name.size ())
	  && ((i = upper_bound (names.begin (), names.end (), word)) != names.end ())
	  && (i-- != names.begin ())
	  && (word == *i)) {
      while (!isalnum (name[pos]))
	 ++pos;

      work = work.substr (pos);
      word = getWord (work);
   }
   TRACE3 ("Words::removeName (const Glib::ustring&) - " << name << "->" << work);
   TRACE1 ("Words::removeName (const Glib::ustring&) - " << name << "->" << work);
}
