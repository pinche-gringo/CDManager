//PROJECT     : CDManager
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 27.11.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2009 - 2011

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


#include <locale.h>

#include <sstream>

#include <boost/tokenizer.hpp>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "CDType.h"

#include "Writer.h"


#if WITH_FILMS == 1
//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
FilmWriter::~FilmWriter () {
}


//-----------------------------------------------------------------------------
/// Substitution of column values
/// \param ctrl: Control character to subsitute
/// \param extend: Flag, if extended substitution is wanted
/// \returns std::string: Substituted string
//-----------------------------------------------------------------------------
std::string FilmWriter::getSubstitute (const char ctrl, bool extend) const {
   if (hFilm) {
      Check2 (hDirector);

      switch (ctrl) {
      case 'n':
	 return YGP::TableWriter::changeHTMLSpecialChars (hFilm->getName ());
	 break;

      case 'y':
	 return hFilm->getYear ().toString ();

      case 'g':
	 Check3 (hFilm->getGenre () < genres.size ());
	 return YGP::TableWriter::changeHTMLSpecialChars (genres.getGenre (hFilm->getGenre ()));

      case 'd':
	 return YGP::TableWriter::changeHTMLSpecialChars (hDirector->getName ());

      case 't':
	 return YGP::TableWriter::changeHTMLSpecialChars (CDType::getInstance ()[hFilm->getType ()]);

      case 'l': {
	 std::string output (addLanguageLinks (hFilm->getLanguage ()));
	 if (hFilm->getTitles ().size ()) {
	    output += " &ndash; ";
	    output += addLanguageLinks (hFilm->getTitles ());
	 }
	 return output;
      }
      } // endswitch
   }
   else {
      Check2 (hDirector);
      Check3 (!hFilm);

      if (ctrl == 'n')
	 return YGP::TableWriter::changeHTMLSpecialChars (hDirector->getName ());
   }
   return std::string (1, ctrl);
}

//-----------------------------------------------------------------------------
/// Writes a film into the table
/// \param film: Film to write
/// \param director: Director of film
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void FilmWriter::writeFilm (const HFilm& film, const HDirector& director, std::ostream& out) {
   Check2 (!hFilm); Check2 (!hDirector);
   hFilm = film;
   hDirector = director;

   std::string value;
   out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\" title=\""
       << YGP::TableWriter::changeHTMLSpecialChars (hFilm->getDescription ()) << "\">";
   oddLine = !oddLine;
   while ((value = getNextNode ()).size ())
      out << "<td>" << value << "</td>";
   out << "</tr>\n";

   hFilm.reset ();
   hDirector.reset ();
}

//-----------------------------------------------------------------------------
/// Writes a director into the table
/// \param director: Director to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void FilmWriter::writeDirector (const HDirector& director, std::ostream& out) {
   Check2 (!hFilm); Check2 (!hDirector);
   hDirector = director;
   out << "<tr><td>&nbsp;" << rowEnd << "\n"
       << "<tr><td colspan=\"5\" class=\"owner\">" << hDirector->getName () << rowEnd;
   hDirector.reset ();

   oddLine = true;
}

//-----------------------------------------------------------------------------
/// Appends the links to the language-flags for the passed languages
/// \param languages: List of languages (comma-separated)
/// \returns HTML-text of links to languages
//-----------------------------------------------------------------------------
std::string FilmWriter::addLanguageLinks (const std::string& languages) {
   std::string output;

   boost::tokenizer<boost::char_separator<char> > langs (languages,
							 boost::char_separator<char> (","));
   for (boost::tokenizer<boost::char_separator<char> >::iterator i (langs.begin ());
	i != langs.end (); ++i) {
      output += "<img src=\"images/";
      output += *i;
      output += ".png\" alt=\"";
      output += *i;
      output += " \">";
   } // endwhile
   return output;
}
#endif


#if WITH_RECORDS == 1
//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RecordWriter::~RecordWriter () {
}


//-----------------------------------------------------------------------------
/// Substitution of column values
/// \param ctrl: Control character to subsitute
/// \param extend: Flag, if extended substitution is wanted
/// \returns std::string: Substituted string
//-----------------------------------------------------------------------------
std::string RecordWriter::getSubstitute (const char ctrl, bool extend) const {
   if (hRecord) {
      Check2 (hInterpret);

      switch (ctrl) {
      case 'n':
	 return YGP::TableWriter::changeHTMLSpecialChars (hRecord->getName ());
	 break;

      case 'y':
	 return hRecord->getYear ().toString ();

      case 'g':
	 Check3 (hRecord->getGenre () < genres.size ());
	 return YGP::TableWriter::changeHTMLSpecialChars (genres.getGenre (hRecord->getGenre ()));

      case 'd':
	 return YGP::TableWriter::changeHTMLSpecialChars (hInterpret->getName ());
      } // endswitch
   }
   else {
      Check2 (hInterpret);
      Check3 (!hRecord);

      if (ctrl == 'n')
	 return YGP::TableWriter::changeHTMLSpecialChars (hInterpret->getName ());
      return "";
   }
   return std::string (1, ctrl);
}

//-----------------------------------------------------------------------------
/// Writes a record into the table
/// \param record: Record to write
/// \param interpret: Interpret of record
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void RecordWriter::writeRecord (const HRecord& record, const HInterpret& interpret,
			      std::ostream& out) {
   Check2 (!hRecord); Check2 (!hInterpret);
   hRecord = record;
   hInterpret = interpret;

   std::string value;
   out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\">";
   oddLine = !oddLine;
   while ((value = getNextNode ()).size ())
      out << "<td>" << value << "</td>";
   out << "</tr>\n";

   hRecord.reset ();
   hInterpret.reset ();
}

//-----------------------------------------------------------------------------
/// Writes a interpret into the table
/// \param interpret: Interpret to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void RecordWriter::writeInterpret (const HInterpret& interpret, std::ostream& out) {
   Check2 (!hRecord); Check2 (!hInterpret);
   hInterpret = interpret;
   out << "<tr><td>&nbsp;" << rowEnd << "\n"
       << "<tr><td colspan=\"3\" class=\"owner\">" << hInterpret->getName () << rowEnd;
   hInterpret.reset ();

   oddLine = true;
}
#endif
