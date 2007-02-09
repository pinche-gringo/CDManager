#ifndef WRITER_H
#define WRITER_H

//$Id: Writer.h,v 1.11 2007/02/09 12:15:00 markus Rel $

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

#include <string>

#if WITH_MOVIES == 1
#  include "Movie.h"
#  include "Director.h"
#endif
#if WITH_RECORDS == 1
#  include "Record.h"
#  include "Interpret.h"
#endif

#include "Genres.h"
#include "Options.h"

#include <YGP/TableWriter.h>


#if WITH_MOVIES == 1
/**Class to write movies as HTML-tables
 */
class MovieWriter : public YGP::TableWriter {
 public:
   MovieWriter (const std::string& format, Genres& genres)
      : YGP::TableWriter (format, TBLW_HTML_PARAMS), oddLine (true), genres (genres) { }
   virtual ~MovieWriter ();

   void writeMovie    (const HMovie& movie, const HDirector& director, std::ostream& out);
   void writeDirector (const HDirector& director, std::ostream& out);

 protected:
   virtual std::string getSubstitute (const char ctrl, bool extend = false) const;

 private:
   MovieWriter (const MovieWriter& other);
   const MovieWriter& operator= (const MovieWriter& other);

   static std::string addLanguageLinks (const std::string& languages);

   HMovie    hMovie;
   HDirector hDirector;

   bool oddLine;
   Genres& genres;
};
#endif


#if WITH_RECORDS == 1
/**Class to write records as HTML-tables
 */
class RecordWriter : public YGP::TableWriter {
 public:
   RecordWriter (const std::string& format, Genres& genres)
      : YGP::TableWriter (format, TBLW_HTML_PARAMS), oddLine (true), genres (genres) { }
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
   Genres& genres;
};
#endif

#endif
