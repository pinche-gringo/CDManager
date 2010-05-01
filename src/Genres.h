#ifndef GENRES_H
#define GENRES_H

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


#include <vector>

#include <glibmm/ustring.h>

#include <YGP/Exception.h>


/**Class to handle the genres of both records and movies.
 */
class Genres {
 public:
   Genres () { }
   virtual ~Genres () { }

   static void loadFromFile (const char* file, Genres& records, Genres& movies,
			     const char* languages) throw (YGP::ParseError, YGP::FileError);

   int getId (const Glib::ustring& genre) const;

   /// Returns the number of genres
   /// \returns std::vector::size_type Number of genres
   size_t size () const { return genres.size (); }

   /// Returns the nth genre
   /// \param genre Number of genre to return
   Glib::ustring getGenre (unsigned int genre) const { return genres[genre]; }

 private:
   Genres (const Genres& other);
   const Genres& operator= (const Genres& other);

   std::vector<Glib::ustring> genres;
};

#endif
