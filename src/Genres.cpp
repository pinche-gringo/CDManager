//$Id: Genres.cpp,v 1.1 2005/01/13 22:28:19 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : libCDMgr
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 13.01.2005
//COPYRIGHT   : Copyright (C) 2005

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


#include <YGP/INIFile.h>

#include <XGP/XAttribute.h>

#include "Genres.h"


//-----------------------------------------------------------------------------
/// Loads the genres from a data-file
/// file: File to load the data from
/// records: Object, to load the record genres into
/// movie: Object, to load the movie genres into
//-----------------------------------------------------------------------------
void Genres::loadFromFile (const char* file, Genres& records, Genres& movies) {
   YGP::INIFile _inifile_ (file);
   YGP::INIList<Glib::ustring, std::map<unsigned int, Glib::ustring> >
      lstMovies ("Movies", movies);
   _inifile_.addSection (lstMovies);
   YGP::INIList<Glib::ustring, std::map<unsigned int, Glib::ustring> >
      lstRecords ("Records", records);
   _inifile_.addSection (lstRecords);

   _inifile_.read ();
}
