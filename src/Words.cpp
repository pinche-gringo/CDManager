//$Id: Words.cpp,v 1.9 2005/09/05 17:52:30 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Words
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.9 $
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

#include <sys/shm.h>

#include <cerrno>
#include <cctype>
#include <cstring>
#include <algorithm>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Trace.h>
#include <YGP/Check.h>

#include "Words.h"


static const unsigned int PAGE_SIZE (4096);

Words::keys* Words::_keys (NULL);


//-----------------------------------------------------------------------------
/// Creates the memory for the reserved words.
/// \param words: Minimal number of reserved words
/// \throw Glib::ustring: Describing text in case of error
/// \remarks The words are stored in shared memory (to be accessible by other
///    processes
//-----------------------------------------------------------------------------
void Words::create (unsigned int words) throw (Glib::ustring) {
   TRACE9 ("Words::create (unsigned int) - " << words);
   if (_keys)
      return;

   unsigned int key (-1U);
   unsigned int size (PAGE_SIZE);
   if ((sizeof (keys) + (sizeof (char*) * words)) > size)
      size = ((sizeof (char*) * words) + PAGE_SIZE + sizeof (keys)) & ~(PAGE_SIZE - 1);

   if (((key = shmget (IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0600)) == -1U)
       || ((_keys = (keys*)(shmat (key, 0, 0))) == (keys*)-1)
       || ((_keys->values = NULL),
	   ((key = shmget (IPC_PRIVATE, size << 1, IPC_CREAT | IPC_EXCL | 0600)) == -1U))
       || ((_keys->values = (char*)(shmat (key, 0, 0))) == (char*)-1)) {
      Glib::ustring error (_("Error allocating shared memory!\n\nReason: %1"));
      error.replace (error.find ("%1"), 2, strerror (errno));
      destroy ();
      throw (error);
   }

   _keys->cNames = _keys->cArticles = 0;
   _keys->endValues = _keys->values;
   _keys->maxEntries = (size - sizeof (keys)) / sizeof (char*);
}

//-----------------------------------------------------------------------------
/// Gets access to the shared memory with the passed id
/// \param key: ID of shared memory
/// \pre Requires the shared memory to be already created
/// \throw Glib::ustring: Describing text in case of error
//-----------------------------------------------------------------------------
void Words::access (unsigned int key) throw (Glib::ustring) {
   TRACE9 ("Words::access (unsigned int)");
   if (_keys)
      return;

   if (((_keys = (keys*)(shmat (key, 0, 0))) == (keys*)-1)
       || ((_keys->values = NULL),
	   ((_keys->values = (char*)(shmat (key, 0, 0))) == (char*)-1))) {
      Glib::ustring error (_("Error accessing shared memory!\n\nReason: %1"));
      error.replace (error.find ("%1"), 2, strerror (errno));
      destroy ();
      throw (error);
   }
}

//-----------------------------------------------------------------------------
/// Frees the used shared memory
//-----------------------------------------------------------------------------
void Words::destroy () {
   if (_keys->values)
      shmdt (_keys->values);
   if (_keys)
      shmdt (_keys);
   _keys = NULL;
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
   Check2 (start <= end);
   memcpy (_keys->aValues + target, _keys->aValues + start, (end - start + 1) * sizeof (char*));
}

//-----------------------------------------------------------------------------
/// Binary search for the passed words in the passed range
/// \param start: First value to search
/// \param end: Last value to search
/// \param word: Word to search
/// \returns unsigned int: Position where to insert
/// \pre start <= end
/// \requires There must be at least one element in the array
//-----------------------------------------------------------------------------
unsigned int Words::binarySearch (unsigned int start, unsigned int end,
				  const char* word) {
   Check2 (_keys);
   Check2 (start <= end);
   Check2 (end < _keys->maxEntries);

   unsigned int middle (0);

   while ((end - start) > 0 ) {
      middle = start + ((end - start) >> 1);
      Check2 (_keys->aValues[start]); Check2 (_keys->aValues[end]); Check2 (_keys->aValues[middle]);
      Check3 (strcmp (_keys->aValues[start], _keys->aValues[middle]) <= 0);
      Check3 (strcmp (_keys->aValues[end], _keys->aValues[middle]) >= 0);

      if (strcmp (word, _keys->aValues[middle]) < 0)
	 end = middle;
      else
	 start = middle + 1;
   }
   TRACE9 ("Words::binarySearch (2x unsigned int, const char*) - " << _keys->aValues[start]);
   return start;
}

//-----------------------------------------------------------------------------
/// Adds a name to ignore
/// \param word: Word to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addName2Ignore (const Glib::ustring& word, unsigned int pos) {
   TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - " << word << " to " << _keys->cNames);
   Check2 (_keys);

   // Try to respect the hint
   if (pos != POS_UNKNOWN) {
      if (pos > _keys->cNames)
	 pos = _keys->cNames;

      TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Checking pos " << pos);
      if (_keys->cNames) {
	 Check2 (_keys->aValues[pos - 1]);
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Comp: " << strcmp (_keys->aValues[pos - 1], word.c_str ()));
	 if (strcmp (_keys->aValues[pos - 1], word.c_str ()) < 0) {
	    if (pos < _keys->cNames) {
	       Check2 (_keys->aValues[pos]);
	       if (strcmp (_keys->aValues[pos], word.c_str ()) > 0)
		  moveValues (pos, _keys->cNames, pos + 1);
	       else
		  pos = POS_UNKNOWN;
	    }
	 }
	 else
	    pos = POS_UNKNOWN;
      }
   }

   // Hint didn't work or wasn't passed: Search for position to insert
   if (pos == POS_UNKNOWN)
      if (_keys->cNames) {
	 TRACE9 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Search: " << word);
	 if ((pos = binarySearch (0, _keys->cNames - 1, word.c_str ())) < _keys->cNames)
	    moveValues (pos, _keys->cNames - 1, pos + 1);
      }
      else
	 pos = 0;

   TRACE3 ("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Insert into " << pos);
   _keys->aValues[pos] = _keys->endValues;
   memcpy (_keys->endValues, word.c_str (), word.bytes ());
   _keys->endValues += word.bytes () + 1;
   _keys->cNames++;
}

//-----------------------------------------------------------------------------
/// Adds an article to ignore
/// \param word: Article to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addArticle (const Glib::ustring& word, unsigned int pos) {
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article or name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeArticle (const Glib::ustring& name) {
   TRACE9 ("Words::removeArticles (const Glib::ustring&) - " << name);
   Check2 (_keys);

   Glib::ustring word (getWord (name));
   if (word.size () != name.size ()
       && containsWord (0, _keys->cArticles, word)) {
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
   Check2 (_keys);

   Glib::ustring work (name);
   Glib::ustring word (getWord (work));
   while ((word.size () != name.size ())
	  && containsWord (0, _keys->cNames - 1, word)) {
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
   Check2 (_keys);
   Check2 (end < _keys->maxEntries);
   if (start < end) {
      unsigned int pos (binarySearch (start, end, word.c_str ()));
      return ((pos != start) && (word == _keys->aValues[pos - 1]));
   }
   else
      return false;

}
