//$Id: Writer.cpp,v 1.7 2004/12/24 04:11:03 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.7 $
//AUTHOR      : Markus Schwab
//CREATED     : 27.11.2004
//COPYRIGHT   : Copyright (C) 2004

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

#include <fstream>

#include <glibmm/convert.h>

#include <gtkmm/messagedialog.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ADate.h>
#include <YGP/ATStamp.h>
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
/// Creates a file and throws an exception, if it can't be created
/// \param name: Name of file to create
/// \param file: Created stream
/// \throws Glib::ustring: A describing text in case of an error
//-----------------------------------------------------------------------------
void MovieWriter::createFile (const char* name, std::ofstream& file) throw (Glib::ustring) {
   TRACE9 ("CDManager::createFile (const char*, std::ofstream&) - " << name);
   Check1 (name);

   file.open (name);
   if (!file) {
      Glib::ustring msg (_("Can't create file `%1'!\n\nReason: %2."));
      msg.replace (msg.find ("%1"), 2, name);
      msg.replace (msg.find ("%2"), 2, strerror (errno));
      throw msg;
   }
}

//-----------------------------------------------------------------------------
/// Reads the contents of the passed file into the passed variable and performs
/// some substitutions within.
///
/// The substitutions are:
///   - @TITLE@ with the passed title
///   - @TIMESTAMPE@ with the current date/time
///   - @DATE@ with current date
///   - @YEAR@ with the current year
/// \param file: File to read from
/// \param target: Variable receiving the input
/// \param title: Value to substitute @TITLE@ with
/// \returns bool: True, if successful
//-----------------------------------------------------------------------------
bool MovieWriter::readHeaderFile (const char* file, std::string& target,
				  const Glib::ustring& title) {
   TRACE9 ("CDManager::readHeaderFile (const char*) - " << file << " - "
	   << title);
   std::ifstream input (file);
   if (!input)
      return false;

   unsigned int i (512);
   char buffer[i];
   // Read as long as there is data or an error occurs
   while (input.read (buffer, i), input.gcount ())
      target.append (buffer, input.gcount ());

   while ((i = target.find ("@TITLE@")) != std::string::npos)
      target.replace (i, 7, title);

   while ((i = target.find ("@TIMESTAMP@")) != std::string::npos)
      target.replace (i, 11, YGP::ATimestamp::now ().toString ());

   while ((i = target.find ("@DATE@")) != std::string::npos)
      target.replace (i, 6, YGP::ADate::today ().toString ());

   while ((i = target.find ("@YEAR@")) != std::string::npos)
      target.replace (i, 6, YGP::ADate::today ().toString ("%Y"));
   return true;
}

//-----------------------------------------------------------------------------
/// Exports the movies from the passed directors
/// \param opt: Options; contains output files and -directories
/// \param genres: Genres
/// \param directors: Vectors holding the directors
//-----------------------------------------------------------------------------
void MovieWriter::exportMovies (const Options& opt,
				std::map<unsigned int, Glib::ustring> genres,
				std::vector<HDirector>& directors) {
   struct {
      std::string name;
      std::string target;
   } htmlData[] =
      { { opt.getMHeader () },
	 { opt.getMFooter () } };

   for (unsigned int i (0); i < (sizeof (htmlData) / sizeof (*htmlData)); ++i) {
      if (htmlData[i].name.size ()
	  && (htmlData[i].name[0] != YGP::File::DIRSEPARATOR))
	 htmlData[i].name = DATADIR + htmlData[i].name;
      if (htmlData[i].target.size ()
	  && (htmlData[i].target[0] != YGP::File::DIRSEPARATOR))
	 htmlData[i].target = DATADIR + htmlData[i].target;

      if (!readHeaderFile (htmlData[i].name.c_str (), htmlData[i].target,
			   _("Movies (by %1)"))) {
	 Glib::ustring error (_("Error reading header file `%1'!\n\nReason: %2"));
	 error.replace (error.find ("%1"), 2, htmlData[i].name);
	 error.replace (error.find ("%2"), 2, strerror (errno));
	 Gtk::MessageDialog dlg (error, Gtk::MESSAGE_WARNING);
	 dlg.run ();
      }
   }

   std::sort (directors.begin (), directors.end (), &Director::compByName);

   std::ofstream file;
   createFile ((opt.getDirOutput () + "Movies.html").c_str (), file);

   std::string title (htmlData[0].target);
   unsigned int pos;
   while ((pos = title.find ("%1")) != std::string::npos)
      title.replace (pos, 2, _("Director"));
   file << title;

   file << ("<div class=\"header\"><a href=\"Movies-Down.html\">Director</a> | "
	    "<a href=\"Movies-Name.html\">Name</a> | "
	    "|<a href=\"Movies-Year.html\">Year</a> | "
	    "<a href=\"Movies-Genre.html\">Genre</a> | "
	    "<a href=\"Movies-Media.html\">Media</a> | "
	    "<a href=\"Movies-Lang.html\">Languages</a></div>\n");

   MovieWriter writer ("%n|%y|%g|%t|%l", genres);
   writer.printStart (file, "");

   std::vector<HMovie> movies;
   YGP::Relation1_N<HDirector, HMovie>& relMovies
      (*dynamic_cast<YGP::Relation1_N<HDirector, HMovie>*>
       (YGP::RelationManager::getRelation ("movies")));
   for (std::vector<HDirector>::const_iterator i (directors.begin ());
	i != directors.end (); ++i)
      if (relMovies.isRelated (*i)) {
	 writer.writeDirector (*i, file);

	 std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	 Check3 (dirMovies.size ());
	 for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
	      m != dirMovies.end (); ++m) {
	    writer.writeMovie (*m, *i, file);
	    movies.push_back (*m);
	 }
      }
   writer.printEnd (file);
   file << htmlData[1].target;

   // Sort reverse
   file.close ();
   createFile ((opt.getDirOutput () + "Movies-Down.html").c_str (), file);
   file << title;

   file << ("<div class=\"header\"><a href=\"Movies.html\">Director</a> | "
	    "<a href=\"Movies-Name.html\">Name</a> | "
	    "|<a href=\"Movies-Year.html\">Year</a> | "
	    "<a href=\"Movies-Genre.html\">Genre</a> | "
	    "<a href=\"Movies-Media.html\">Media</a> | "
	    "<a href=\"Movies-Lang.html\">Languages</a></div>\n");

   writer.printStart (file, "");
   for (std::vector<HDirector>::reverse_iterator i (directors.rbegin ());
	i != directors.rend (); ++i)
      if (relMovies.isRelated (*i)) {
	 writer.writeDirector (*i, file);

	 std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	 Check3 (dirMovies.size ());
	 for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
	      m != dirMovies.end (); ++m)
	    writer.writeMovie (*m, *i, file);
      }
   writer.printEnd (file);
   file << htmlData[1].target;

   typedef bool (*PFNCOMPARE) (const HMovie&, const HMovie&);
   struct {
      const char* title;
      const char* file;
      const char* filedown;
      const char* format;
      const char* sorted;
      PFNCOMPARE  fnCompare;
   } aOutputs[] =
	 { { "<div class=\"header\"><a href=\"%1\">Name</a></div>"
	     "|<div class=\"header\"><a href=\"Movies.html\">Director</a></div>|"
	     "|<div class=\"header\"><a href=\"Movies-Year.html\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Genre.html\">Genre</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Media.html\">Media</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Lang.html\">Language(s)</a></div>",
	     "Movies-Name.html", "Movies-Namedown.html",
	     "%n|%d|%y|%g|%t|%l", N_("Name"), &Movie::compByName },
	   { "|<div class=\"header\"><a href=\"%1\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Name.html\">Name</a></div></p>"
	     "|<div class=\"header\"><a href=\"Movies.html\">Director</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Genre.html\">Genre</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Media.html\">Media</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Lang.html\">Language(s)</a></div>",
	     "Movies-Year.html", "Movies-Yeardown.html",
	     "%y|%n|%d|%g|%t|%l", N_("Year"), &Movie::compByYear },
	   { "<div class=\"header\"><a href=\"%1\">Genre</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Name.html\">Name</a></div>"
	     "|<div class=\"header\"><a href=\"Movies.html\">Director</a></div>|"
	     "|<div class=\"header\"><a href=\"Movies-Year.html\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Media.html\">Media</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Lang.html\">Language(s)</a></div>",
	     "Movies-Genre.html", "Movies-Genredown.html",
	     "%g|%n|%d|%y|%t|%l", N_("Genre"), &Movie::compByGenre },
	   { "<div class=\"header\"><a href=\"%1\">Media</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Name.html\">Name</a></div>"
	     "|<div class=\"header\"><a href=\"Movies.html\">Director</a></div>|"
	     "|<div class=\"header\"><a href=\"Movies-Year.html\">Year</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Genre.html\">Genre</a></div>|"
	     "<div class=\"header\"><a href=\"Movies-Lang.html\">Language(s)</a></div>",
	     "Movies-Media.html", "Movies-Mediadown.html",
	     "%t|%n|%d|%y|%g|%l", N_("Media"), &Movie::compByMedia } };

   for (unsigned int i (0); i < (sizeof (aOutputs) / sizeof (*aOutputs)); ++i) {
      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].file).c_str (), file);
      title = htmlData[0].target;
      while ((pos = title.find ("%1")) != std::string::npos)
	 title.replace (pos, 2, _(aOutputs[i].sorted));
      file << title;

      std::sort (movies.begin (), movies.end (), aOutputs[i].fnCompare);

      std::string header (aOutputs[i].title);
      header.replace (header.find ("%1"), 2, aOutputs[i].filedown);
      MovieWriter writer (aOutputs[i].format, genres);
      writer.printStart (file, header);

      for (std::vector<HMovie>::const_iterator m (movies.begin ());
	   m != movies.end (); ++m) {
	 HDirector director;
	 director = relMovies.getParent (*m); Check3 (director.isDefined ());
	 writer.writeMovie (*m, director, file);
      }

      writer.printEnd (file);
      file << htmlData[1].target;

      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].filedown).c_str (), file);
      file << title;

      header = aOutputs[i].title;
      header.replace (header.find ("%1"), 2, aOutputs[i].file);
      writer.printStart (file, header);

      for (std::vector<HMovie>::reverse_iterator m (movies.rbegin ());
	   m != movies.rend (); ++m) {
	 HDirector director;
	 director = relMovies.getParent (*m); Check3 (director.isDefined ());
	 writer.writeMovie (*m, director, file);
      }

      writer.printEnd (file);
      file << htmlData[1].target;
   }

   // Export by language
   file.close ();
   createFile ((opt.getDirOutput () + "Movies-Lang.html").c_str (), file);
   title = htmlData[0].target;
   while ((pos = title.find ("%1")) != std::string::npos)
      title.replace (pos, 2, _("Language"));
   file << title;

   file << ("<div class=\"header\"><a href=\"Movies-Name.html\">Name</a> | "
	    "<a href=\"Movies.html\">Director</a> | "
	    "<a href=\"Movies-Year.html\">Year</a> | "
	    "<a href=\"Movies-Genre.html\">Genre</a> | "
	    "<a href=\"Movies-Media.html\">Media</a></div>\n");

   file << "<div class=\"header\">|";
   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l) {
      file << " <a href=\"#" << l->first << "\"><img src=\"images/" << l->first
	   << ".png\"> " << l->second.getInternational () << "</a> |";
   }
   file << "</div>";

   MovieWriter langWriter ("%l|%n|%d||%y|%g|%t", genres);
   std::sort (movies.begin (), movies.end (), &Movie::compByName);

   writer.printStart (file, "");

   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l) {
      file << "<tr><td colspan=\"6\"><div class=\"header\"><a name=\"" << l->first << "\">\n<br><h2>"
	   << l->second.getNational () << '/' << l->second.getInternational ()
	   << "</div></h2></td></tr>";

      file << ("<tr><td><div class=\"header\">&nbsp;</div></td>"
	       "<td><div class=\"header\"><a href=\"Movies-Name.html\">Name</a></div></td>"
	       "<td><div class=\"header\"><a href=\"Movies.html\">Director</a></div></td>"
	       "<td><div class=\"header\"><a href=\"Movies-Year.html\">Year</a></div></td>"
	       "<td><div class=\"header\"><a href=\"Movies-Genre.html\">Genre</a></div></td>"
	       "<td><div class=\"header\"><a href=\"Movies-Media.html\">Media(s)</a></div></td></tr>");

      for (std::vector<HMovie>::const_iterator m (movies.begin ());
	   m != movies.end (); ++m)
	 if (((*m)->getLanguage ().find (l->first) != std::string::npos)
	     || ((*m)->getTitles ().find (l->first) != std::string::npos)) {
	    HDirector director;
	    director = relMovies.getParent (*m); Check3 (director.isDefined ());
	    langWriter.writeMovie (*m, director, file);
	 }
   }

   langWriter.printEnd (file);
   file << htmlData[1].target;
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
}
