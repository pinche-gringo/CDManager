//$Id: Writer.cpp,v 1.13 2005/10/28 21:41:00 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.13 $
//AUTHOR      : Markus Schwab
//CREATED     : 27.11.2004
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

#include <locale.h>

#include <sstream>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Tokenize.h>

#include "CDType.h"

#include "Writer.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MovieWriter::~MovieWriter () {
}


//-----------------------------------------------------------------------------
/// Substitution of column values
/// \param ctrl: Control character to subsitute
/// \param extend: Flag, if extended substitution is wanted
/// \returns std::string: Substituted string
//-----------------------------------------------------------------------------
std::string MovieWriter::getSubstitute (const char ctrl, bool extend) const {
   if (hMovie.isDefined ()) {
      Check2 (hDirector.isDefined ());

      switch (ctrl) {
      case 'n':
	 return YGP::TableWriter::changeHTMLSpecialChars (hMovie->getName ());
	 break;

      case 'y':
	 return hMovie->getYear ().toString ();

      case 'g':
	 Check3 (genres.find (hMovie->getGenre ()) != genres.end ());
	 return YGP::TableWriter::changeHTMLSpecialChars (genres.find (hMovie->getGenre ())->second);

      case 'd':
	 return YGP::TableWriter::changeHTMLSpecialChars (hDirector->getName ());

      case 't':
	 return YGP::TableWriter::changeHTMLSpecialChars (CDType::getInstance ()[hMovie->getType ()]);

      case 'l': {
	 std::string output (addLanguageLinks (hMovie->getLanguage ()));
	 if (hMovie->getTitles ().size ()) {
	    output += " &ndash; ";
	    output += addLanguageLinks (hMovie->getTitles ());
	 }
	 return output;
      }
      } // endswitch
   }
   else {
      Check2 (hDirector.isDefined ());
      Check3 (!hMovie.isDefined ());

      if (ctrl == 'n')
	 return YGP::TableWriter::changeHTMLSpecialChars (hDirector->getName ());
   }
   return std::string (1, ctrl);
}

//-----------------------------------------------------------------------------
/// Writes a movie into the table
/// \param movie: Movie to write
/// \param director: Director of movie
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void MovieWriter::writeMovie (const HMovie& movie, const HDirector& director,
			      std::ostream& out) {
   Check2 (!hMovie.isDefined ()); Check2 (!hDirector.isDefined ());
   hMovie = movie;
   hDirector = director;

   std::string value;
   out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\">";
   oddLine = !oddLine;
   while (!((value = getNextNode ()).empty ()))
      out << "<td>" << value << "</td>";
   out << "</tr>\n";

   hMovie.undefine ();
   hDirector.undefine ();
}

//-----------------------------------------------------------------------------
/// Writes a director into the table
/// \param director: Director to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void MovieWriter::writeDirector (const HDirector& director, std::ostream& out) {
   Check2 (!hMovie.isDefined ()); Check2 (!hDirector.isDefined ());
   hDirector = director;
   out << "<tr><td>&nbsp;" << rowEnd << "\n"
       << "<tr><td colspan=\"5\" class=\"owner\">" << hDirector->getName () << rowEnd;
   hDirector.undefine ();

   oddLine = true;
}

//-----------------------------------------------------------------------------
/// Appends the links to the language-flags for the passed languages
/// \param languages: List of languages (comma-separated)
/// \returns HTML-text of links to languages
//-----------------------------------------------------------------------------
std::string MovieWriter::addLanguageLinks (const std::string& languages) {
   std::string output;
   YGP::Tokenize langs (languages);
   while (langs.getNextNode (',').size ()) {
      output += "<img src=\"images/";
      output += langs.getActNode ();
      output += ".png\" alt=\"";
      output += langs.getActNode ();
      output += " \">";
   } // endwhile
   return output;
}


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
   if (hRecord.isDefined ()) {
      Check2 (hInterpret.isDefined ());

      switch (ctrl) {
      case 'n':
	 return YGP::TableWriter::changeHTMLSpecialChars (hRecord->getName ());
	 break;

      case 'y':
	 return hRecord->getYear ().toString ();

      case 'g':
	 Check3 (genres.find (hRecord->getGenre ()) != genres.end ());
	 return YGP::TableWriter::changeHTMLSpecialChars (genres.find (hRecord->getGenre ())->second);

      case 'd':
	 return YGP::TableWriter::changeHTMLSpecialChars (hInterpret->getName ());
      } // endswitch
   }
   else {
      Check2 (hInterpret.isDefined ());
      Check3 (!hRecord.isDefined ());

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
   Check2 (!hRecord.isDefined ()); Check2 (!hInterpret.isDefined ());
   hRecord = record;
   hInterpret = interpret;

   std::string value;
   out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\">";
   oddLine = !oddLine;
   while (!((value = getNextNode ()).empty ()))
      out << "<td>" << value << "</td>";
   out << "</tr>\n";

   hRecord.undefine ();
   hInterpret.undefine ();
}

//-----------------------------------------------------------------------------
/// Writes a interpret into the table
/// \param interpret: Interpret to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void RecordWriter::writeInterpret (const HInterpret& interpret, std::ostream& out) {
   Check2 (!hRecord.isDefined ()); Check2 (!hInterpret.isDefined ());
   hInterpret = interpret;
   out << "<tr><td>&nbsp;" << rowEnd << "\n"
       << "<tr><td colspan=\"3\" class=\"owner\">" << hInterpret->getName () << rowEnd;
   hInterpret.undefine ();

   oddLine = true;
}
