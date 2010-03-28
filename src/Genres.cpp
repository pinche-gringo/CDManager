//$Id: Genres.cpp,v 1.8 2007/03/05 19:38:36 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : libCDMgr
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.8 $
//AUTHOR      : Markus Schwab
//CREATED     : 13.01.2005
//COPYRIGHT   : Copyright (C) 2005 - 2007, 2009, 2010

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

#include <boost/tokenizer.hpp>

#include <glibmm/convert.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>

#include <XGP/XAttribute.h>

#include "Genres.h"


//-----------------------------------------------------------------------------
/// Loads the genres from a data-file. The parameter \languages specifies the
/// language to use.
/// \param file: File to load the data from
/// \param records: Object, to load the record genres into
/// \param movie: Object, to load the movie genres into
/// \param languages: Colon-separated list of languages
/// \throw YGP::ParseError, YGP::FileError: In case of an error
//-----------------------------------------------------------------------------
void Genres::loadFromFile (const char* file, Genres& records, Genres& movies,
			   const char* languages) throw (YGP::ParseError, YGP::FileError) {
   Check1 (file); Check1 (languages);
   std::string name (file);

   // Check every language-entry (while removing trailing specifiers)
   std::string langs (languages);
   boost::tokenizer<boost::char_separator<char> > ext (langs, boost::char_separator<char> (":"));
   std::string extension;
   struct stat sfile;
   for (boost::tokenizer<boost::char_separator<char> >::iterator i (ext.begin ());
	i != ext.end (); ++i) {
      extension = *i;
      std::string search;
      do {
	 search = name + std::string (1, '.') + extension;

	 TRACE9 ("Genres::loadFromFile (...) - Trying " << search);
	 if (!::stat (search.c_str (), &sfile) && (sfile.st_mode & S_IFREG))
	    break;

	 size_t pos (extension.rfind ('_'));
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
   YGP::INIList<Glib::ustring, std::vector<Glib::ustring> > lstMovies ("Movies", movies.genres);
   _inifile_.addSection (lstMovies);
   YGP::INIList<Glib::ustring, std::vector<Glib::ustring> > lstRecords ("Records", records.genres);
   _inifile_.addSection (lstRecords);

   _inifile_.read ();
}

//-----------------------------------------------------------------------------
/// Gets the id of the passed genre
/// \param Name of genre to convert to its associated number
/// \returns int ID of genre or -1
//-----------------------------------------------------------------------------
int Genres::getId (const Glib::ustring& genre) const {
   std::vector<Glib::ustring>::const_iterator g (std::find (genres.begin (), genres.end (), genre));
   return (g != genres.end ()) ? (g - genres.begin ()) : -1;
}
