//$Id: Genres.cpp,v 1.5 2006/01/23 03:15:07 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : libCDMgr
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.5 $
//AUTHOR      : Markus Schwab
//CREATED     : 13.01.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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


#include <sys/stat.h>

#include <string>

#include <glibmm/convert.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>
#include <YGP/Tokenize.h>

#include <XGP/XAttribute.h>

#include "Genres.h"


//-----------------------------------------------------------------------------
/// Loads the genres from a data-file. The parameter \languages specifies the
/// language to use.
/// file: File to load the data from
/// records: Object, to load the record genres into
/// movie: Object, to load the movie genres into
/// languages: Colon-separated list of languages
//-----------------------------------------------------------------------------
void Genres::loadFromFile (const char* file, Genres& records, Genres& movies,
			   const char* languages) throw (std::string) {
   Check1 (file); Check1 (languages);
   std::string name (file);

   // Check every language-entry (while removing trailing specifiers)
   YGP::Tokenize ext (languages);
   std::string extension;
   struct stat sfile;
   while ((extension = ext.getNextNode (':')).size ()) {
      std::string search;
      do {
	 search = name + std::string (1, '.') + extension;

	 TRACE9 ("Genres::loadFromFile (...) - Trying " << search);
	 if (!::stat (search.c_str (), &sfile) && (sfile.st_mode & S_IFREG))
	    break;

	 unsigned int pos (extension.rfind ('_'));
	 if (pos == std::string::npos)
	    pos = 0;
	 extension.replace (pos, extension.length (), 0, '\0');
      } while (extension.size ());

      if (extension.size ()) {
	 TRACE1 ("Genres::loadFromFile (...) - Using " << search);
	 name = search;
	 break;
      }
   } // end-while

   YGP::INIFile _inifile_ (name.c_str ());
   YGP::INIList<Glib::ustring, std::map<unsigned int, Glib::ustring> >
      lstMovies ("Movies", movies);
   _inifile_.addSection (lstMovies);
   YGP::INIList<Glib::ustring, std::map<unsigned int, Glib::ustring> >
      lstRecords ("Records", records);
   _inifile_.addSection (lstRecords);

   _inifile_.read ();
}
