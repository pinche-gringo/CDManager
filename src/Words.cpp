//$Id: Words.cpp,v 1.15 2006/02/09 20:40:32 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Words
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.15 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006

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

#include <sys/shm.h>

#include <map>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <algorithm>

#include <YGP/Trace.h>
#include <YGP/Check.h>
#include <YGP/Process.h>

#include "Words.h"


#undef PAGE_SIZE
static const unsigned int PAGE_SIZE (4096);

int Words::_key (-1);
static std::map<pid_t, Words::values*> ptrs;


//-----------------------------------------------------------------------------
/// Creates the memory for the reserved words.
/// \param words: Minimal number of reserved words
/// \throw Glib::ustring: Describing text in case of error
/// \remarks The words are stored in shared memory (to be accessible by other
///    processes
//-----------------------------------------------------------------------------
void Words::create (unsigned int words) throw (Glib::ustring) {
   TRACE8 ("Words::create (unsigned int) - " << words);
   if ((_key != -1) || (ptrs.find (YGP::Process::getPID ()) != ptrs.end ()))
      return;

   Words::values* shMem (NULL);
   unsigned int size (PAGE_SIZE);
   if ((sizeof (values) + (sizeof (char*) * words)) > size)
      size = ((sizeof (char*) * words) + PAGE_SIZE + sizeof (values)) & ~(PAGE_SIZE - 1);

   if (((_key = shmget (IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0600)) == -1)
       || ((shMem = (values*)(shmat (_key, 0, 0))) == (values*)-1)
       || ((shMem->values = NULL),
	   ((shMem->valuesKey = shmget (IPC_PRIVATE, size << 1, IPC_CREAT | IPC_EXCL | 0600)) == -1))
       || ((shMem->values = (char*)(shmat (shMem->valuesKey, 0, 0))) == (char*)-1)) {
      destroy ();
      throw (Glib::ustring (strerror (errno)));
   }

   shMem->cNames = shMem->cArticles = 0;
   shMem->endValues = shMem->values;
   shMem->maxEntries = (size - sizeof (values)) / sizeof (char*);
   TRACE1 ("Words::create (unsigned int) - Key: " << _key);
   ptrs[YGP::Process::getPID ()] = shMem;
}

//-----------------------------------------------------------------------------
/// Gets access to the shared memory with the passed id
/// \param key: ID of shared memory
/// \pre Requires the shared memory to be already created
/// \throw Glib::ustring: Describing text in case of error
//-----------------------------------------------------------------------------
void Words::access (unsigned int key) throw (Glib::ustring) {
   TRACE8 ("Words::access (unsigned int) - " << key);
   if (!key || (ptrs.find (YGP::Process::getPID ()) != ptrs.end ()))
      return;

   Words::values* shMem (NULL);
   if (((shMem = (values*)(shmat (key, 0, 0))) == (values*)-1)
       || ((shMem->values = NULL),
	   ((shMem->values = (char*)(shmat (shMem->valuesKey, 0, 0))) == (char*)-1))) {
      destroy ();
      throw (Glib::ustring (strerror (errno)));
   }
   TRACE9 ("Words::access (unsigned int) - Articles: " << shMem->cArticles << "; Names: "
	   << shMem->cNames << ": " << *shMem->values);
   ptrs[YGP::Process::getPID ()] = shMem;
}

//-----------------------------------------------------------------------------
/// Frees the used shared memory
//-----------------------------------------------------------------------------
void Words::destroy () {
   Check3 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());
   Words::values* shMem (ptrs[YGP::Process::getPID ()]);

   if (shMem && shMem != (values*)-1) {
      if (shMem->values && shMem->values != (char*)-1)
	 shmdt (shMem->values);
      shmdt (shMem);
   }
   ptrs.erase (ptrs.find (YGP::Process::getPID ()));
   if (ptrs.empty ())
      _key = -1;
}


//-----------------------------------------------------------------------------
/// Moves values from the values-array to another position
/// \param start: Start position
/// \param end: End position
/// \param target: Target position
/// \pre start <= end
//-----------------------------------------------------------------------------
void Words::moveValues (unsigned int start, unsigned int end, unsigned int target) {
   TRACE9 ("Words::moveValues (3x unsigned int start) - [" << start << '-' << end
	   << "] -> " << target << "; Bytes: " << (end - start + 1) * sizeof (char*));
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);
   Check2 (start <= end);
   memcpy (shMem->aOffsets + target, shMem->aOffsets + start, (end - start + 1) * sizeof (char*));
}

//-----------------------------------------------------------------------------
/// Binary search for the passed words in the passed range
/// \param values: Values to search
/// \param start: First value to search
/// \param end: Last value to search
/// \param word: Word to search
/// \returns unsigned int: Position where to insert
/// \pre start <= end
/// \requires There must be at least one element in the array
//-----------------------------------------------------------------------------
unsigned int Words::binarySearch (values* values, unsigned int start,
				  unsigned int end, const char* word) {
   Check1 (values);
   Check2 (end < values->maxEntries);

   unsigned int middle (0);

   while ((end - start) > 0 ) {
      middle = start + ((end - start) >> 1);
      Check3 (strcmp (values->values + values->aOffsets[start], values->values + values->aOffsets[middle]) <= 0);
      Check3 (strcmp (values->values + values->aOffsets[end], values->values + values->aOffsets[middle]) >= 0);

      if (strcmp (word, values->values + values->aOffsets[middle]) < 0)
	 end = middle;
      else
	 start = middle + 1;
   }
   TRACE9 ("Words::binarySearch (values*, 2x unsigned int, const char*) - " << values->values + values->aOffsets[start]);
   return start;
}

//-----------------------------------------------------------------------------
/// Adds a name to ignore
/// \param word: Word to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addName2Ignore (const Glib::ustring& word, unsigned int pos) {
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);
   TRACE2 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->cNames);

   // Try to respect the hint
   if (pos != POS_UNKNOWN) {
      if (pos > shMem->cNames)
	 pos = shMem->cNames;

      TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Checking pos " << pos);
      if (shMem->cNames) {
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Comp: " << strcmp (shMem->values + shMem->aOffsets[pos - 1], word.c_str ()));
	 if (strcmp (shMem->values + shMem->aOffsets[pos - 1], word.c_str ()) < 0) {
	    if (pos < shMem->cNames) {
	       if (strcmp (shMem->values + shMem->aOffsets[pos], word.c_str ()) <= 0)
		  pos = POS_UNKNOWN;
	    }
	 }
	 else
	    pos = POS_UNKNOWN;
      }
   }

   // Hint didn't work or wasn't passed: Search for position to insert
   if (pos == POS_UNKNOWN)
      if (shMem->cNames) {
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Search: " << word);
	 pos = binarySearch (shMem, 0, shMem->cNames - 1, word.c_str ());
      }
      else
	 pos = 0;

   if (pos < shMem->cNames)
      moveValues (pos, shMem->cNames, pos + 1);

   TRACE3 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Insert into " << pos);
   shMem->aOffsets[pos] = shMem->endValues - shMem->values;
   memcpy (shMem->endValues, word.c_str (), word.bytes ());
   shMem->endValues += word.bytes () + 1;
   shMem->cNames++;
}

//-----------------------------------------------------------------------------
/// Adds an article to ignore
/// \param word: Article to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addArticle (const Glib::ustring& word, unsigned int pos) {
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);
   TRACE2 ("Words::addArticle (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->cArticles);

   // Try to respect the hint
   if (pos != POS_UNKNOWN) {
      pos = ((pos > shMem->cArticles) ? shMem->maxEntries - 1
	     : shMem->maxEntries - shMem->cArticles + pos);

      TRACE9 ("Words::addArticle (const Glib::ustring&, unsigned int) - Checking pos " << pos);
      if (shMem->cArticles) {
	 TRACE9 ("Words::addArticle (const Glib::ustring&, unsigned int) - Comp: " << strcmp (shMem->values + shMem->aOffsets[pos], word.c_str ()));
	 if (strcmp (shMem->values + shMem->aOffsets[pos], word.c_str ()) < 0) {
	    if (pos < (shMem->maxEntries - 1)) {
	       if (strcmp (shMem->values + shMem->aOffsets[pos + 1], word.c_str ()) <= 0)
		  pos = POS_UNKNOWN;
	    }
	 }
	 else
	    pos = POS_UNKNOWN;
      }
   }

   // Hint didn't work or wasn't passed: Search for position to insert
   if (pos == POS_UNKNOWN)
      if (shMem->cArticles) {
	 TRACE9 ("Words::addArticles (const Glib::ustring&, unsigned int) - Search: " << word);
	 pos = binarySearch (shMem, shMem->maxEntries - shMem->cArticles,
			     shMem->maxEntries - 1, word.c_str ());
      }
      else
	 pos = shMem->maxEntries - 1;

   if (pos >= (shMem->maxEntries - shMem->cArticles))
      moveValues (shMem->maxEntries - shMem->cArticles, pos,
		  shMem->maxEntries - shMem->cArticles - 1);

   TRACE3 ("Words::addArticle (const Glib::ustring&, unsigned int) - Insert into " << pos);
   shMem->aOffsets[pos] = shMem->endValues - shMem->values;
   memcpy (shMem->endValues, word.c_str (), word.bytes ());
   shMem->endValues += word.bytes () + 1;
   shMem->cArticles++;
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article or name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeArticle (const Glib::ustring& name) {
   TRACE9 ("Words::removeArticles (const Glib::ustring&) - " << name);
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);

   Glib::ustring word (getWord (name));
   if (word.size () != name.size ()
       && containsWord (shMem->maxEntries - shMem->cArticles, shMem->maxEntries - 1, word)) {
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
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);
   Glib::ustring work (name);
   Glib::ustring word (getWord (work));
   while ((word.size () != name.size ())
	  && containsWord (0, shMem->cNames - 1, word)) {
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
/// Checks if the passed list contains the passed word
/// \param start: Start value
/// \param end: End value
/// \param word: Word to search for
/// \returns bool: True, if the word exists
//-----------------------------------------------------------------------------
bool Words::containsWord (unsigned int start, unsigned int end, const Glib::ustring& word) {
   TRACE9 ("Words::containsWord (2x unsigned int, const Glib::ustring& word) - [" << start << '-' << end << ']');
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]);
   Check2 (end < shMem->maxEntries);
   Check2 (start <= end);
   TRACE9 ("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->aOffsets[start]);
   TRACE9 ("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->aOffsets[end]);
   if (start < end) {
      unsigned int pos (binarySearch (shMem, start, end, word.c_str ()));
      return ((pos != start) && (word == (shMem->values + shMem->aOffsets[pos - 1])));
   }
   else
      return false;

}

//-----------------------------------------------------------------------------
/// Returns the number of articles stored
/// \returns unsigned int: Number of articles stored
//-----------------------------------------------------------------------------
unsigned int Words::cArticles () {
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());
   return ptrs[YGP::Process::getPID ()]->cArticles;
}

//-----------------------------------------------------------------------------
/// Returns the number of names stored
/// \returns unsigned int: Number of names stored
//-----------------------------------------------------------------------------
unsigned int Words::cNames () {
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());
   return ptrs[YGP::Process::getPID ()]->cNames;
}

//-----------------------------------------------------------------------------
/// Returns the stored values for the current process
/// \returns unsigned int: Stored values
//-----------------------------------------------------------------------------
Words::values* Words::getValues () {
   Check2 (ptrs.find (YGP::Process::getPID ()) != ptrs.end ());
   return ptrs[YGP::Process::getPID ()];
}
