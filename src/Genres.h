#ifndef GENRES_H
#define GENRES_H

//$Id: Genres.h,v 1.3 2006/01/23 03:15:07 markus Rel $

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


#include <map>

#include <glibmm/ustring.h>


/**Class to handle the genres of both records and movies.
 */
class Genres : public std::map<unsigned int, Glib::ustring> {
 public:
   Genres () { }
   virtual ~Genres () { }

   static void loadFromFile (const char* file, Genres& records, Genres& movies,
			     const char* languages) throw (std::string);

 private:
   Genres (const Genres& other);
   const Genres& operator= (const Genres& other);
};

#endif
