#ifndef WRITER_H
#define WRITER_H

//$Id: Writer.h,v 1.1 2004/11/28 01:05:38 markus Rel $

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
#include <string>

#include <Movie.h>
#include <Director.h>

#include <YGP/TableWriter.h>


class MovieWriter : public YGP::HTMLWriter {
 public:
   MovieWriter (const std::string& format,
		std::map<unsigned int, Glib::ustring> genres)
      : YGP::HTMLWriter (format), genres (genres) { }
   virtual ~MovieWriter ();

   void writeMovie    (const HMovie& movie, std::ostream& out);
   void writeDirector (const HDirector& director, std::ostream& out);

 protected:
   virtual std::string getSubstitute (const char ctrl, bool extend = false) const;

 private:
   MovieWriter (const MovieWriter& other);
   const MovieWriter& operator= (const MovieWriter& other);

   HMovie    hMovie;
   HDirector hDirector;

   std::map<unsigned int, Glib::ustring> genres;
};

#endif
