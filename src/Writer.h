#ifndef WRITER_H
#define WRITER_H

//$Id: Writer.h,v 1.3 2004/12/05 03:30:00 markus Exp $

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

#include "Movie.h"
#include "Record.h"
#include "Director.h"
#include "Interpret.h"

#include <YGP/TableWriter.h>


/**Class to write movies as HTML-tables
 */
class MovieWriter : public YGP::HTMLWriter {
 public:
   MovieWriter (const std::string& format,
		std::map<unsigned int, Glib::ustring> genres)
      : YGP::HTMLWriter (format), oddLine (true), genres (genres) { }
   virtual ~MovieWriter ();

   void writeMovie    (const HMovie& movie, const HDirector& director, std::ostream& out);
   void writeDirector (const HDirector& director, std::ostream& out);

 protected:
   virtual std::string getSubstitute (const char ctrl, bool extend = false) const;

 private:
   MovieWriter (const MovieWriter& other);
   const MovieWriter& operator= (const MovieWriter& other);

   HMovie    hMovie;
   HDirector hDirector;

   bool oddLine;
   std::map<unsigned int, Glib::ustring> genres;
};


/**Class to write records as HTML-tables
 */
class RecordWriter : public YGP::HTMLWriter {
 public:
   RecordWriter (const std::string& format,
		 std::map<unsigned int, Glib::ustring> genres)
      : YGP::HTMLWriter (format), oddLine (true), genres (genres) { }
   virtual ~RecordWriter ();

   void writeRecord    (const HRecord& record, const HInterpret& interpret, std::ostream& out);
   void writeInterpret (const HInterpret& interpret, std::ostream& out);

 protected:
   virtual std::string getSubstitute (const char ctrl, bool extend = false) const;

 private:
   RecordWriter (const RecordWriter& other);
   const RecordWriter& operator= (const RecordWriter& other);

   HRecord    hRecord;
   HInterpret hInterpret;

   bool oddLine;
   std::map<unsigned int, Glib::ustring> genres;
};

#endif
