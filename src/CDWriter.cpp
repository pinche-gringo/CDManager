//$Id: CDWriter.cpp,v 1.1 2005/01/10 02:10:42 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDWriter
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 07.01.2005
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


#include <cdmgr-cfg.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <glibmm/ustring.h>
#include <glibmm/convert.h>

#include <gtkmm/messagedialog.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ADate.h>
#include <YGP/ATStamp.h>
#include <YGP/Relation.h>

#include "DB.h"
#include "Movie.h"
#include "Writer.h"
#include "Director.h"

#include "CDWriter.h"
#include "Options.meta"


const YGP::IVIOApplication::longOptions CDWriter::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "version", 'V' },
   { "recHeader", 'r' },
   { "recFooter", 'R' },
   { "movieHeader", 'm' },
   { "movieFooter", 'M' },
   { "outputDir", 'd' },
   { NULL, '\0' } };


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDWriter::~CDWriter () {
   TRACE9 ("CDWriter::~CDWriter ()");
}


//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void CDWriter::showHelp () const {
   std::cout << _("<SHORT DESCRIPTION>\n\nUsage: ") << PACKAGE_NAME
             << _(" [OPTIONS] [LANGUAGE-ID]\n\n")
	     << "  -d, --outputDir ..... " << _("Directory to export data to\n")
	     << "  -r, --recHeader ..... " << _("File to use as header for records\n")
	     << "  -r, --recFooter ..... " << _("File to use as footer for records\n")
	     << "  -r, --movieHeader ... " << _("File to use as header for movies")
	     << "  -r, --movieFooter ... " << _("File to use as footer for movies\n")
             << "  -V, --version ....... " << _("Output version information and exit\n")
             << "  -h, -?, --help ...... " << _("Displays this help and exit\n\n");
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require :
///     option not '\0´'
//-----------------------------------------------------------------------------
bool CDWriter::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
   case 'd': {
      const char* pDir (getOptionValue ());
      if (pDir)
         opt.setDirOutput (pDir);
      else
         std::cerr << PACKAGE << _("-warning: No directory specified! Ignoring option `d'\n");
      break; }

   case 'm':
   case 'M':
   case 'R':
   case 'r': {
      const char* pFile (getOptionValue ());
      if (pFile)
	 ((option == 'm') ? opt.setMHeader (pFile)
	  : ((option == 'M') ?  opt.setMFooter (pFile)
	     : ((option == 'r') ?  opt.setRHeader (pFile)
		: opt.setRFooter  (pFile))));
      else {
	 Glib::ustring e (_("-warning: No file specified! Ignoring option `%1'\n"));
	 e.replace (e.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << e;
      }
      break; }

   case 'V':
      std::cout << description () << '\n';
      exit (0);
      break;

   default:
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDWriter::perform (int argc, const char** argv) {
   opt.setDirOutput ("tmp/");

   if (argc != 1) {
      std::cerr << PACKAGE << _("-error: Need language id as parameter");
      return - 1;
   }

   std::map<unsigned int, Glib::ustring> genres;
#if 0
   Database::store ("SELECT id, genre FROM MGenres");
   while (Database::hasData ()) {
      // Fill and store artist entry from DB-values
      genres[Database::getResultColumnAsUInt (0)] =
	 Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

      Database::getNextResultRow ();
   }
#else
   genres[0] = "Undefined";
   genres[1] = "Comedy";
   genres[5] = "Drama";
#endif

   Glib::ustring transTitle (_("Movies (by %1)"));
   Glib::ustring transDirector (_("Director"));
   Glib::ustring transName (_("Name"));
   Glib::ustring transYear (_("Year"));
   Glib::ustring transGenre (_("Genre"));
   Glib::ustring transMedia (_("Media"));
   Glib::ustring transLang (_("Language(s)"));

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
			   transTitle)) {
	 Glib::ustring error (_("Error reading header file `%1'!\n\nReason: %2"));
	 error.replace (error.find ("%1"), 2, htmlData[i].name);
	 error.replace (error.find ("%2"), 2, strerror (errno));
	 Gtk::MessageDialog dlg (error, Gtk::MESSAGE_WARNING);
	 dlg.run ();
      }
   }

   std::ofstream file;
   createFile ((opt.getDirOutput () + "Movies.html." + argv[0]).c_str (), file);

   std::string title (htmlData[0].target);
   unsigned int pos (0);
   while ((pos = title.find ("%1", pos)) != std::string::npos)
      title.replace (pos, 2, _("Director"));
   file << title;

   file << "<div class=\"header\"><a href=\"Movies-Down.html\">"
	<< transDirector << "</a> | <a href=\"Movies-Name.html\">"
	<< transName << "</a> | |<a href=\"Movies-Year.html\">"
	<< transYear << "</a> | <a href=\"Movies-Genre.html\">"
	<< transGenre << "</a> | <a href=\"Movies-Media.html\">"
	<< transMedia << "</a> | <a href=\"Movies-Lang.html\">"
	<< transLang << "</a></div>\n";

   MovieWriter writer ("%n|%y|%g|%t|%l", genres);
   writer.printStart (file, "");
   file.flush ();

   HMovie movie;
   HDirector director;
   std::vector<HDirector> directors;
   std::vector<HMovie> movies;
   YGP::Relation1_N<HDirector, HMovie> relMovies ("movies");

   char type;
   std::cin >> type;
   while (!std::cin.eof ()) {
      switch (type) {
      case 'D':
	 director.define ();
	 director->readFromStream (std::cin);
	 TRACE9 ("CDWriter::perform (int, const char**) - Director: " << director->getName ());
	 directors.push_back (director);
	 break;

      case 'M':
	 Check3 (director.isDefined ());
	 movie.define ();
	 std::cin >> *movie;
	 TRACE9 ("CDWriter::perform (int, const char**) - Movie: " << movie->getName ());
	 if (!relMovies.isRelated (director))
	    writer.writeDirector (director, file);

	 relMovies.relate (director, movie);
	 writer.writeMovie (movie, director, file);
	 movies.push_back (movie);
	 break;

      default:
	 Check3 (0);
	 return -1;
      }

      std::cin >> type;
   } // end-while not eof

   writer.printEnd (file);
   file << htmlData[1].target;

   // Sort reverse
   file.close ();
   createFile ((opt.getDirOutput () + "Movies-Down.html." + argv[0]).c_str (), file);
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
      createFile ((opt.getDirOutput () + aOutputs[i].file + "." + argv[0]).c_str (), file);
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
   return 0;
}

//-----------------------------------------------------------------------------
/// Returns a short description of the program (not the help!)
/// \returns const char*: Pointer to a short description
//-----------------------------------------------------------------------------
const char* CDWriter::description () const {
   static std::string version =
      (PACKAGE_NAME " V" VERSION " - "
       + std::string (_("Compiled on"))
       + std::string (" " __DATE__ " - " __TIME__ "\n\n")
       + std::string (_("Author: Markus Schwab; e-mail: g17m0@lycos.com"
			"\nDistributed under the terms of the GNU General "
			"Public License")));
   return version.c_str ();
 }

//-----------------------------------------------------------------------------
/// Creates a file and throws an exception, if it can't be created
/// \param name: Name of file to create
/// \param file: Created stream
/// \throws Glib::ustring: A describing text in case of an error
//-----------------------------------------------------------------------------
void CDWriter::createFile (const char* name, std::ofstream& file) throw (Glib::ustring) {
   TRACE9 ("CDWriter::createFile (const char*, std::ofstream&) - " << name);
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
bool CDWriter::readHeaderFile (const char* file, std::string& target,
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
/// Entrypoint of application
/// \param argc: Number of parameters
/// \param argv: Array with pointer to parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main (int argc, const char* argv[]) {
   CDWriter::initI18n (PACKAGE, LOCALEDIR);
   CDWriter appl (argc, argv);
   return appl.run ();
}
