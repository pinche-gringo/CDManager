//PROJECT     : CDManager
//SUBSYSTEM   : Words
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009, 2010

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

typedef struct WordPtrs {
   Words::values* info;
   char*          values;

   WordPtrs () : info (NULL), values (NULL) { }
} WordPtrs;
static std::map<pid_t, WordPtrs*> ptrs;


//-----------------------------------------------------------------------------
/// Creates the memory for the reserved words.
/// \param words: Minimal number of reserved words
/// \throw std::invalid_argument: Describing text in case of error
/// \remarks The words are stored in shared memory (to be accessible by other
///    processes
//-----------------------------------------------------------------------------
void Words::create (unsigned int words) throw (std::invalid_argument) {
   TRACE8 ("Words::create (unsigned int) - " << words);
   if ((_key != -1) || areAvailable ())
      return;

   WordPtrs* shMem (new WordPtrs);
   ptrs[YGP::Process::getPID ()] = shMem;

   unsigned int size (PAGE_SIZE);
   if ((sizeof (values) + (sizeof (char*) * words)) > size)
      size = ((sizeof (char*) * words) + PAGE_SIZE + sizeof (values)) & ~(PAGE_SIZE - 1);

   if (((_key = shmget (IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0600)) == -1)
       || ((shMem->info = (values*)(shmat (_key, 0, 0))) == (values*)-1)
       || ((shMem->info->valuesKey = shmget (IPC_PRIVATE, size << 1, IPC_CREAT | IPC_EXCL | 0600)) == -1)
       || ((shMem->values = (char*)(shmat (shMem->info->valuesKey, 0, 0))) == (char*)-1)) {
      destroy ();
      throw (std::invalid_argument (strerror (errno)));
   }

   shMem->info->cNames = shMem->info->cArticles = 0;
   shMem->info->used = 0;
   shMem->info->maxEntries = (size - sizeof (values)) / sizeof (char*);
   TRACE1 ("Words::create (unsigned int) - Key: " << _key);
}

//-----------------------------------------------------------------------------
/// Gets access to the shared memory with the passed id
/// \param key: ID of shared memory
/// \pre Requires the shared memory to be already created
/// \throw std::invalid_argument: Describing text in case of error
//-----------------------------------------------------------------------------
void Words::access (unsigned int key) throw (std::invalid_argument) {
   TRACE8 ("Words::access (unsigned int) - " << key);
   if (!key || areAvailable ())
      return;

   WordPtrs* shMem (new WordPtrs);
   ptrs[YGP::Process::getPID ()] = shMem;

   if (((shMem->info = (values*)(shmat (key, 0, 0))) == (values*)-1)
       || ((shMem->values = (char*)(shmat (shMem->info->valuesKey, 0, 0))) == (char*)-1)) {
      destroy ();
      throw (std::invalid_argument (strerror (errno)));
   }
   TRACE9 ("Words::access (unsigned int) - Articles: " << shMem->info->cArticles << "; Names: "
	   << shMem->info->cNames << ": " << *shMem->values);
}

//-----------------------------------------------------------------------------
/// Checks if the Words are already available for the actual process
/// \returns bool: True, if the Words are available
//-----------------------------------------------------------------------------
bool Words::areAvailable () {
   return ptrs.find (YGP::Process::getPID ()) != ptrs.end ();
}

//-----------------------------------------------------------------------------
/// Frees the used shared memory
//-----------------------------------------------------------------------------
void Words::destroy () {
   Check3 (areAvailable ());
   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);

   if (shMem->info && shMem->info != (values*)-1) {
      if (shMem->values && shMem->values != (char*)-1)
	 shmdt (shMem->values);
      shmdt (shMem->info);
   }

   delete ptrs[YGP::Process::getPID ()];
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
   Check2 (areAvailable ());

   Words::values* shMem (ptrs[YGP::Process::getPID ()]->info);
   Check2 (start <= end);
   memcpy (shMem->aOffsets + target, shMem->aOffsets + start, (end - start + 1) * sizeof (char*));
}

//-----------------------------------------------------------------------------
/// Binary search for the passed words in the passed range
/// \param values: Values to search
/// \param data: Stored data
/// \param start: First value to search
/// \param end: Last value to search
/// \param word: Word to search
/// \returns unsigned int: Position where to insert
/// \pre start <= end
/// \requires There must be at least one element in the array
//-----------------------------------------------------------------------------
unsigned int Words::binarySearch (values* values, char* data, unsigned int start,
				  unsigned int end, const char* word) {
   Check1 (values);
   Check2 (end <= values->maxEntries);

   unsigned int middle (0);

   while ((end - start) > 0 ) {
      middle = start + ((end - start) >> 1);
      Check3 (strcmp (data + values->aOffsets[start], data + values->aOffsets[middle]) <= 0);

      if (strcmp (word, data + values->aOffsets[middle]) < 0)
	 end = middle;
      else
	 start = middle + 1;
   }
   TRACE9 ("Words::binarySearch (values*, char*, 2x unsigned int, const char*) - " << data + values->aOffsets[start]);
   return start;
}

//-----------------------------------------------------------------------------
/// Adds a name to ignore
/// \param word: Word to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addName2Ignore (const Glib::ustring& word, unsigned int pos) {
   Check2 (areAvailable ());

   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);
   TRACE2 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->info->cNames);

   // Try to respect the hint
   if (pos != POS_UNKNOWN) {
      if (pos > shMem->info->cNames)
	 pos = shMem->info->cNames;

      TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Checking pos " << pos);
      if (shMem->info->cNames) {
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Comp: " << strcmp (shMem->values + shMem->info->aOffsets[pos - 1], word.c_str ()));
	 if (strcmp (shMem->values + shMem->info->aOffsets[pos - 1], word.c_str ()) < 0) {
	    if (pos < shMem->info->cNames) {
	       if (strcmp (shMem->values + shMem->info->aOffsets[pos], word.c_str ()) <= 0)
		  pos = POS_UNKNOWN;
	    }
	 }
	 else
	    pos = POS_UNKNOWN;
      }
   }

   // Hint didn't work or wasn't passed: Search for position to insert
   if (pos == POS_UNKNOWN) {
      if (shMem->info->cNames) {
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Search: " << word);
	 pos = binarySearch (shMem->info, shMem->values, 0, shMem->info->cNames, word.c_str ());
      }
      else
	 pos = 0;
   }

   if (pos < shMem->info->cNames)
      moveValues (pos, shMem->info->cNames, pos + 1);

   TRACE3 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Insert into " << pos);
   shMem->info->aOffsets[pos] = shMem->info->used;
   memcpy (shMem->values + shMem->info->used, word.c_str (), word.bytes ());
   shMem->info->used += word.bytes () + 1;
   shMem->info->cNames++;
}

//-----------------------------------------------------------------------------
/// Adds an article to ignore
/// \param word: Article to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addArticle (const Glib::ustring& word, unsigned int pos) {
   Check2 (areAvailable ());

   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);
   TRACE2 ("Words::addArticle (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->info->cArticles);

   // Try to respect the hint
   if (pos != POS_UNKNOWN) {
      pos = ((pos > shMem->info->cArticles) ? shMem->info->maxEntries - 1
	     : shMem->info->maxEntries - shMem->info->cArticles + pos);

      TRACE9 ("Words::addArticle (const Glib::ustring&, unsigned int) - Checking pos " << pos);
      if (shMem->info->cArticles) {
	 TRACE9 ("Words::addArticle (const Glib::ustring&, unsigned int) - Comp: " << strcmp (shMem->values + shMem->info->aOffsets[pos], word.c_str ()));
	 if (strcmp (shMem->values + shMem->info->aOffsets[pos], word.c_str ()) < 0) {
	    if (pos < (shMem->info->maxEntries - 1)) {
	       if (strcmp (shMem->values + shMem->info->aOffsets[pos + 1], word.c_str ()) <= 0)
		  pos = POS_UNKNOWN;
	    }
	 }
	 else
	    pos = POS_UNKNOWN;
      }
   }

   // Hint didn't work or wasn't passed: Search for position to insert
   if (pos == POS_UNKNOWN) {
      if (shMem->info->cArticles) {
	 TRACE9 ("Words::addArticles (const Glib::ustring&, unsigned int) - Search: " << word);
	 pos = binarySearch (shMem->info, shMem->values, shMem->info->maxEntries - shMem->info->cArticles,
			     shMem->info->maxEntries, word.c_str ());
      }
      else
	 pos = shMem->info->maxEntries - 1;
   }

   if (pos >= (shMem->info->maxEntries - shMem->info->cArticles))
      moveValues (shMem->info->maxEntries - shMem->info->cArticles, pos,
		  shMem->info->maxEntries - shMem->info->cArticles - 1);

   TRACE3 ("Words::addArticle (const Glib::ustring&, unsigned int) - Insert into " << pos);
   shMem->info->aOffsets[pos] = shMem->info->used;
   memcpy (shMem->values + shMem->info->used, word.c_str (), word.bytes ());
   shMem->info->used += word.bytes () + 1;
   shMem->info->cArticles++;
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article or name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeArticle (const Glib::ustring& name) {
   TRACE9 ("Words::removeArticles (const Glib::ustring&) - " << name);
   Check2 (areAvailable ());

   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);

   Glib::ustring word (getWord (name));
   if (word.size () != name.size ()
       && containsWord (shMem->info->maxEntries - shMem->info->cArticles, shMem->info->maxEntries, word)) {
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
   Check2 (areAvailable ());

   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);
   Glib::ustring work (name);
   Glib::ustring word (getWord (work));
   while ((word.size () != name.size ())
	  && containsWord (0, shMem->info->cNames, word)) {
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
   Check2 (areAvailable ());

   WordPtrs* shMem (ptrs[YGP::Process::getPID ()]);
   Check2 (end <= shMem->info->maxEntries);
   Check2 (start < end);
   TRACE9 ("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->info->aOffsets[start]);
   TRACE9 ("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->info->aOffsets[end]);
   if (start < end) {
      unsigned int pos (binarySearch (shMem->info, shMem->values, start, end, word.c_str ()));
      return ((pos != start) && (word == (shMem->values + shMem->info->aOffsets[pos - 1])));
   }
   else
      return false;

}

//-----------------------------------------------------------------------------
/// Returns the number of articles stored
/// \returns unsigned int: Number of articles stored
//-----------------------------------------------------------------------------
unsigned int Words::cArticles () {
   Check2 (areAvailable ());
   return ptrs[YGP::Process::getPID ()]->info->cArticles;
}

//-----------------------------------------------------------------------------
/// Returns the number of names stored
/// \returns unsigned int: Number of names stored
//-----------------------------------------------------------------------------
unsigned int Words::cNames () {
   Check2 (areAvailable ());
   return ptrs[YGP::Process::getPID ()]->info->cNames;
}

//-----------------------------------------------------------------------------
/// Returns the stored words for the current process
/// \returns unsigned int: Stored words
//-----------------------------------------------------------------------------
const char* Words::getValues () {
   Check2 (areAvailable ());
   return ptrs[YGP::Process::getPID ()]->values;
}

//-----------------------------------------------------------------------------
/// Returns the stored values for the current process
/// \returns unsigned int: Stored values
//-----------------------------------------------------------------------------
Words::values* Words::getInfo () {
   Check2 (areAvailable ());
   return ptrs[YGP::Process::getPID ()]->info;
}
