#ifndef WORDS_H
#define WORDS_H

//$Id: Words.h,v 1.1 2004/11/29 18:35:23 markus Rel $

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


#include <vector>

#include <glibmm/ustring.h>


/**Class to store some words
 */
class Words {
 public:
   static bool cArticles () { return articles.size (); }
   static bool cNames () { return names.size (); }
   static void addName2Ignore (const Glib::ustring& word) {
      names.push_back (word); }
   static void init ();

   static Glib::ustring removeArticle (const Glib::ustring& name);
   static Glib::ustring removeNames (const Glib::ustring& name);

 private:
   //Prohibited manager functions
   Words ();
   ~Words ();
   Words (const Words&);
   Words& operator= (const Words&);

   static Glib::ustring getWord (const Glib::ustring& name);

   static std::vector<Glib::ustring> articles;
   static std::vector<Glib::ustring> names;
};

#endif
