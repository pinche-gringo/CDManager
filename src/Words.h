#ifndef WORDS_H
#define WORDS_H

//$Id: Words.h,v 1.3 2005/01/18 03:56:49 markus Rel $

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


namespace Gtk {
   class Widget;
}


/**Class to store some words
 */
class Words {
   friend class WordDialog;

 public:
   static unsigned int cArticles () { return articles.size (); }
   static unsigned int cNames () { return names.size (); }
   static void addName2Ignore (const Glib::ustring& word) {
      names.push_back (word); }
   static void addArticle (const Glib::ustring& word) {
      articles.push_back (word); }
   static void sortNames ();
   static void sortArticles ();

   static Glib::ustring removeArticle (const Glib::ustring& name);
   static Glib::ustring removeNames (const Glib::ustring& name);

   static Gtk::Widget* makeDialog ();
   static void commitDialogData (Gtk::Widget* dialog);

   static std::vector<Glib::ustring>::const_iterator namesEnd () { return names.end (); }
   static std::vector<Glib::ustring>::const_iterator namesBegin () { return names.begin (); }
   static std::vector<Glib::ustring>::const_iterator articlesEnd () { return articles.end (); }
   static std::vector<Glib::ustring>::const_iterator articlesBegin () { return articles.begin (); }

 private:
   //Prohibited manager functions
   Words ();
   ~Words ();
   Words (const Words&);
   Words& operator= (const Words&);

   static bool containsWord (const std::vector<Glib::ustring>& list,
			     const Glib::ustring& word);

   static Glib::ustring getWord (const Glib::ustring& name);
   static bool compare (const Glib::ustring& w1, const Glib::ustring& w2);

   static std::vector<Glib::ustring> articles;
   static std::vector<Glib::ustring> names;
};

#endif
