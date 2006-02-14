#ifndef WORDS_H
#define WORDS_H

//$Id: Words.h,v 1.9 2006/02/14 16:31:45 markus Rel $

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


#include <glibmm/ustring.h>


/**Class to store some words. These words are stored in a shared memory
 * segment.
 */
class Words {
   friend class WordDialog;

 public:
   /// \name Management methods
   //@{
   static void create (unsigned int words = 1000) throw (Glib::ustring);
   static void access (unsigned int key) throw (Glib::ustring);
   static void destroy ();
   //@}

   static unsigned int cArticles ();
   static unsigned int cNames ();

   enum { POS_END = -2U, POS_UNKNOWN = -1U };
   static void addName2Ignore (const Glib::ustring& word, unsigned int pos = POS_UNKNOWN);
   static void addArticle (const Glib::ustring& word, unsigned int pos = POS_UNKNOWN);

   static Glib::ustring removeArticle (const Glib::ustring& name);
   static Glib::ustring removeNames (const Glib::ustring& name);

   /// Call a callback for each specified name
   static void forEachName (unsigned int start, unsigned int end,
			    void (*cb) (const char*)) {
      values* shMem (getInfo ());
      for (unsigned int i (start); i < end; ++i)
	 cb (getValues () + shMem->aOffsets[i]);
   }
   template <class T>
   static void forEachName (unsigned int start, unsigned int end, T& obj,
			    void (T::* cb) (const char*)) {
      values* shMem (getInfo ());
      for (unsigned int i (start); i < end; ++i)
	 (obj.*cb) (getValues () + shMem->aOffsets[i]);
   }
   /// Call a callback for each specified article
   static void forEachArticle (unsigned int start, unsigned int end,
			       void (*cb) (const char*)) {
      values* shMem (getInfo ());
      for (unsigned int i (start); i < end; ++i)
	 cb (getValues () + shMem->aOffsets[shMem->maxEntries - shMem->cArticles + i]);
   }
   /// Call a callback for each specified article
   template <class T>
   static void forEachArticle (unsigned int start, unsigned int end, T& obj,
			       void (T::* cb) (const char*)) {
      values* shMem (getInfo ());
      for (unsigned int i (start); i < end; ++i)
	 (obj.*cb) (getValues () + shMem->aOffsets[shMem->maxEntries - shMem->cArticles + i]);
   }

   static int getMemoryKey () { return _key; }

   typedef struct {
      int            valuesKey;
      unsigned int   cNames;
      unsigned int   cArticles;
      unsigned int   maxEntries;
      unsigned int   used;
      unsigned short aOffsets[];
   } values;

 private:
   //Prohibited manager functions
   ~Words ();
   Words (const Words&);
   Words& operator= (const Words&);

   static int   _key;

   static values* getInfo ();
   static const char* getValues ();

   static unsigned int binarySearch (values* values, char* data, unsigned int start, unsigned int end, const char* word);
   static void moveValues (unsigned int start, unsigned int end, unsigned int target);

   static Glib::ustring Words::getWord (const Glib::ustring& text);
   static bool containsWord (unsigned int start, unsigned int end, const Glib::ustring& word);
};

#endif
