//$Id: Writer.cpp,v 1.8 2005/01/12 22:48:50 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.8 $
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

#include <glibmm/convert.h>

#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ADate.h>
#include <YGP/ATStamp.h>
#include <YGP/Process.h>
#include <YGP/Relation.h>
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
	 return changeSpecialChars (Glib::locale_from_utf8 (hMovie->getName ()));
	 break;

      case 'y':
	 return hMovie->getYear ().toString ();

      case 'g':
	 Check3 (genres.find (hMovie->getGenre ()) != genres.end ());
	 return changeSpecialChars (Glib::locale_from_utf8 (genres.find (hMovie->getGenre ())->second));

      case 'd':
	 return changeSpecialChars (Glib::locale_from_utf8 (hDirector->getName ()));

      case 't':
	 return changeSpecialChars (Glib::locale_from_utf8 (CDType::getInstance ()[hMovie->getType ()]));

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
	 return changeSpecialChars (Glib::locale_from_utf8 (hDirector->getName ()));
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
   out << "<tr><td>&nbsp;</td></tr>\n"
       << "<tr><td colspan=\"5\" class=\"owner\">" << hDirector->getName () << "</td></tr>\n";
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
      output += "\">";
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
	 return changeSpecialChars (Glib::locale_from_utf8 (hRecord->getName ()));
	 break;

      case 'y':
	 return hRecord->getYear ().toString ();

      case 'g':
	 Check3 (genres.find (hRecord->getGenre ()) != genres.end ());
	 return changeSpecialChars (Glib::locale_from_utf8 (genres.find (hRecord->getGenre ())->second));

      case 'd':
	 return changeSpecialChars (Glib::locale_from_utf8 (hInterpret->getName ()));
      } // endswitch
   }
   else {
      Check2 (hInterpret.isDefined ());
      Check3 (!hRecord.isDefined ());

      if (ctrl == 'n')
	 return changeSpecialChars (Glib::locale_from_utf8 (hInterpret->getName ()));
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
   out << "<tr><td>&nbsp;</td></tr>\n"
       << "<tr><td colspan=\"3\" class=\"owner\">" << hInterpret->getName () << "</td></tr>\n";
   hInterpret.undefine ();

   oddLine = true;
}

//-----------------------------------------------------------------------------
/// Exports the movies from the passed directors
/// \param opt: Options; contains output files and -directories
/// \param genres: Genres
/// \param artists: Vectors holding the interprets
//-----------------------------------------------------------------------------
void RecordWriter::exportRecords (const Options& opt,
				  std::map<unsigned int, Glib::ustring> genres,
				  std::vector<HInterpret>& artists) {
#if 0
   struct {
      const std::string name;
      std::string       target;
   } htmlData[] =
      { { DATADIR + opt.getRHeader () },
	 { DATADIR + opt.getRFooter () } };

   for (unsigned int i (0); i < (sizeof (htmlData) / sizeof (*htmlData)); ++i)
      if (!readHeaderFile (htmlData[i].name.c_str (), htmlData[i].target,
			   _("Records (by %1)"))) {
	 Glib::ustring error (_("Error reading header file `%1'!\n\nReason: %2"));
	 error.replace (error.find ("%1"), 2, htmlData[i].name);
	 error.replace (error.find ("%2"), 2, strerror (errno));
	 Gtk::MessageDialog dlg (error, Gtk::MESSAGE_WARNING);
	 dlg.run ();
      }

   std::sort (artists.begin (), artists.end (), &Interpret::compByName);

   std::ofstream file;
   createFile ((opt.getDirOutput () + "Records.html").c_str (), file);
   std::string title (htmlData[0].target);
   unsigned int pos;
   while ((pos = title.find ("%1")) != std::string::npos)
      title.replace (pos, 2, _("Interpret"));
   file << title;

   file << ("<div class=\"header\"><a href=\"Records-Down.html\">Interpret</a> | "
	    "<a href=\"Records-Name.html\">Name</a> | "
	    "|<a href=\"Records-Year.html\">Year</a> | "
	    "<a href=\"Records-Genre.html\">Genre</a></div>\n");

   RecordWriter writer ("%n|%y|%g", genres);
   writer.printStart (file, "");

   std::vector<HRecord> records;
   YGP::Relation1_N<HInterpret, HRecord>& relRecords
      (*dynamic_cast<YGP::Relation1_N<HInterpret, HRecord>*>
       (YGP::RelationManager::getRelation ("records")));
   for (std::vector<HDirector>::const_iterator i (artists.begin ());
	i != artists.end (); ++i)
      if (relRecords.isRelated (*i)) {
	 writer.writeInterpret (*i, file);

	 std::vector<HRecord>& dirRecords (relRecords.getObjects (*i));
	 Check3 (dirRecords.size ());
	 for (std::vector<HRecord>::const_iterator m (dirRecords.begin ());
	      m != dirRecords.end (); ++m) {
	    writer.writeRecord (*m, *i, file);
	    records.push_back (*m);
	 }
      }
   writer.printEnd (file);
   file << htmlData[1].target;

   // Sort reverse
   file.close ();
   createFile ((opt.getDirOutput () + "Records-Down.html").c_str (), file);
   file << title;

   file << ("<div class=\"header\"><a href=\"Records.html\">Interpret</a> | "
	    "<a href=\"Records-Name.html\">Name</a> | "
	    "|<a href=\"Records-Year.html\">Year</a> | "
	    "<a href=\"Records-Genre.html\">Genre</a></div>\n");

   writer.printStart (file, "");
   for (std::vector<HInterpret>::reverse_iterator i (artists.rbegin ());
	i != artists.rend (); ++i)
      if (relRecords.isRelated (*i)) {
	 writer.writeInterpret (*i, file);

	 std::vector<HRecord>& dirRecords (relRecords.getObjects (*i));
	 Check3 (dirRecords.size ());
	 for (std::vector<HRecord>::const_iterator m (dirRecords.begin ());
	      m != dirRecords.end (); ++m)
	    writer.writeRecord (*m, *i, file);
      }
   writer.printEnd (file);
   file << htmlData[1].target;

   typedef bool (*PFNCOMPARE) (const HRecord&, const HRecord&);
   struct {
      const char* title;
      const char* file;
      const char* filedown;
      const char* format;
      const char* sorted;
      PFNCOMPARE  fnCompare;
   } aOutputs[] =
	 { { "<div class=\"header\"><a href=\"%1\">Name</a></div>"
	     "|<div class=\"header\"><a href=\"Records.html\">Interpret</a></div>|"
	     "|<div class=\"header\"><a href=\"Records-Year.html\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Records-Genre.html\">Genre</a></div>",
	     "Records-Name.html", "Records-Namedown.html",
	     "%n|%d|%y|%g", N_("Name"), &Record::compByName },
	   { "|<div class=\"header\"><a href=\"%1\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Records-Name.html\">Name</a></div></p>"
	     "|<div class=\"header\"><a href=\"Records.html\">Interpret</a></div>|"
	     "<div class=\"header\"><a href=\"Records-Genre.html\">Genre</a></div>",
	     "Records-Year.html", "Records-Yeardown.html",
	     "%y|%n|%d|%g", N_("Year"), &Record::compByYear },
	   { "<div class=\"header\"><a href=\"%1\">Genre</a></div>|"
	     "<div class=\"header\"><a href=\"Records-Name.html\">Name</a></div>"
	     "|<div class=\"header\"><a href=\"Records.html\">Interpret</a></div>|"
	     "|<div class=\"header\"><a href=\"Records-Year.html\">Year</a></div>",
	     "Records-Genre.html", "Records-Genredown.html",
	     "%g|%n|%d|%y", N_("Genre"), &Record::compByGenre } };

   for (unsigned int i (0); i < (sizeof (aOutputs) / sizeof (*aOutputs)); ++i) {
      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].file).c_str (), file);
      title = htmlData[0].target;
      while ((pos = title.find ("%1")) != std::string::npos)
	 title.replace (pos, 2, _(aOutputs[i].sorted));
      file << title;

      std::sort (records.begin (), records.end (), aOutputs[i].fnCompare);

      std::string header (aOutputs[i].title);
      header.replace (header.find ("%1"), 2, aOutputs[i].filedown);
      RecordWriter writer (aOutputs[i].format, genres);
      writer.printStart (file, header);

      for (std::vector<HRecord>::const_iterator m (records.begin ());
	   m != records.end (); ++m) {
	 HInterpret interpret;
	 interpret = relRecords.getParent (*m); Check3 (interpret.isDefined ());
	 writer.writeRecord (*m, interpret, file);
      }

      writer.printEnd (file);
      file << htmlData[1].target;

      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].filedown).c_str (), file);
      file << title;

      header = aOutputs[i].title;
      header.replace (header.find ("%1"), 2, aOutputs[i].file);
      writer.printStart (file, header);

      for (std::vector<HRecord>::reverse_iterator m (records.rbegin ());
	   m != records.rend (); ++m) {
	 HInterpret interpret;
	 interpret = relRecords.getParent (*m); Check3 (interpret.isDefined ());
	 writer.writeRecord (*m, interpret, file);
      }

      writer.printEnd (file);
      file << htmlData[1].target;
   }
#endif
}
