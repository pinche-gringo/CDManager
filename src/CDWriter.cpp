//$Id: CDWriter.cpp,v 1.6 2005/01/13 22:32:57 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDWriter
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.6 $
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


#define DONT_CONVERT
#include <cdmgr-cfg.h>

#include <cstring>

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

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
#include "Genres.h"
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
	     << "  -R, --recFooter ..... " << _("File to use as footer for records\n")
	     << "  -m, --movieHeader ... " << _("File to use as header for movies\n")
	     << "  -M, --movieFooter ... " << _("File to use as footer for movies\n")
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
/// Writes the header for the table
/// \param lang: Language code; Used for links to documents
/// \param format: Format of header; A sequence of the following characters
///     - d: Director
///     - n: Name of movie
///     - y: Year the movie was made
///     - g: Genre of movie
///     - m: Media containing the movie
///     - l: Language(s)
///     - [: Start of <div>-tag
///     - ]: End of <div>-tag
///     - !: With HMTL column-separaters "</td><td>"
///     - =: With HTML space "&nbsp;"
/// \param upSorted: Flag, if first column is sorted upwards
/// \param stream: Stream to write to
/// \remarks Other characters are just copied
//-----------------------------------------------------------------------------
void CDWriter::writeHeader (const char* lang, const char* format,
			    std::ostream& stream, bool upSorted) {
   TRACE9 ("CDWriter::writeHeader (2x const char*, std::ostream&, bool) - "
	   << lang << "; " << format);
   Check3 (lang); Check3 (format);

   static char formats[] = "dnygml-![]=";
   static char* docs[] = { "", "-Name", "-Year", "-Genre", "-Media", "-Lang",
			   NULL, NULL, NULL, NULL, NULL };
   static std::string titles[] =
      { _("Director"), _("Name"), _("Year"),_("Genre"), _("Media"),
	_("Language(s)"), " | ", "</td><td>", "<div class=\"header\">",
	"</div>", "&nbsp;" };
   Check3 ((sizeof (docs) / sizeof (*docs)) == strlen (formats));
   Check3 ((sizeof (titles) / sizeof (*titles)) == strlen (formats));

   while (*format) {
      char* posFormat (strchr (formats, *format));
      if (posFormat) {
	 unsigned int pos (strchr (formats, *format++) - formats);
	 Check3 (titles[pos].size ());

	 if (docs[pos]) {
	    stream << "<a href=\"Movies" << docs[pos];
	    if (upSorted) {
	       stream << "down";
	       upSorted = false;
	    }
	    stream << ".html." << lang << "\">" << titles[pos] << "</a>";
	 }
	 else
	    stream << titles[pos];
      }
      else
	 stream << *format++;
   }
   stream << '\n';
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDWriter::perform (int argc, const char** argv) {
   TRACE9 ("CDWriter::perform (int, const char**) - " << argc);
   if (argc != 1) {
      std::cerr << PACKAGE << _("-error: Need language id as parameter");
      return - 1;
   }

   Genres movieGenres, recGenres;
   Genres::loadFromFile (DATADIR "Genres.dat", recGenres, movieGenres);

   Glib::ustring transTitle (_("Movies (by %1)"));

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
	 Glib::ustring error (Glib::locale_to_utf8 (_("Error reading header file `%1'!\n\nReason: %2")));
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

   writeHeader (argv[0], "[d-n-y-g-m-l]", file);

   MovieWriter writer ("%n|%y|%g|%t|%l", movieGenres);
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
      try {
	 TRACE4 ("CDWriter::perform (int, char**) - Type: " << type);
	 switch (type) {
	 case 'D':
	    director.define ();
	    std::cin >> *director;
	    TRACE9 ("CDWriter::perform (int, char**) - Director: " << director->getName ());
	    directors.push_back (director);
	    break;

	 case 'M':
	    Check3 (director.isDefined ());
	    movie.define ();
	    std::cin >> *movie;
	    TRACE9 ("CDWriter::perform (int, char**) - Movie: " << movie->getName ());
	    if (!relMovies.isRelated (director)) {
	       writer.writeDirector (director, file);
	       file.flush ();
	    }

	    relMovies.relate (director, movie);
	    writer.writeMovie (movie, director, file);
	    movies.push_back (movie);
	    break;

	 default:
	    Check3 (0);
	    return -1;
	 }
      }
      catch (std::string& error) {
	 std::cerr << "Error: " << error << '\n';
	 break;
      }
      catch (std::exception& error) {
	 std::cerr << "Error: " << error.what () << '\n';
	 break;
      }
      catch (...) {
	 std::cerr << "Unspecified error!\n";
      }

      std::cin >> type;
   } // end-while not eof

   writer.printEnd (file);
   file << htmlData[1].target;

   // Sort reverse
   file.close ();
   createFile ((opt.getDirOutput () + "Moviesdown.html." + argv[0]).c_str (), file);
   file << title;

   writeHeader (argv[0], "[d-n-y-g-m-l]", file, false);

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
      { { "[n]|[d]|[y]|[g]|[m]|[l]", "Movies-Name.html", "Movies-Namedown.html",
	  "%n|%d|%y|%g|%t|%l", N_("Name"), &Movie::compByName },
	{ "[y]|[n]|[d]|[g]|[m]|[l]", "Movies-Year.html", "Movies-Yeardown.html",
	  "%y|%n|%d|%g|%t|%l", N_("Year"), &Movie::compByYear },
	{ "[g]|[n]|[d]|[y]|[m]|[l]", "Movies-Genre.html", "Movies-Genredown.html",
	  "%g|%n|%d|%y|%t|%l", N_("Genre"), &Movie::compByGenre },
	{ "[m]|[n]|[d]|[y]|[g]|[l]", "Movies-Media.html", "Movies-Mediadown.html",
	  "%t|%n|%d|%y|%g|%l", N_("Media"), &Movie::compByMedia } };

   for (unsigned int i (0); i < (sizeof (aOutputs) / sizeof (*aOutputs)); ++i) {
      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].file + "." + argv[0]).c_str (), file);
      title = htmlData[0].target;
      pos = 0;
      while ((pos = title.find ("%1", pos)) != std::string::npos)
	 title.replace (pos, 2, _(aOutputs[i].sorted));

      file << title;

      std::sort (movies.begin (), movies.end (), aOutputs[i].fnCompare);

      MovieWriter writer (aOutputs[i].format, movieGenres);

      { std::stringstream header;
      writeHeader (argv[0], aOutputs[i].title, header);

      writer.printStart (file, header.str ()); }

      for (std::vector<HMovie>::const_iterator m (movies.begin ());
	   m != movies.end (); ++m) {
	 HDirector director;
	 director = relMovies.getParent (*m); Check3 (director.isDefined ());
	 writer.writeMovie (*m, director, file);
      }

      writer.printEnd (file);
      file << htmlData[1].target;

      file.close ();
      createFile ((opt.getDirOutput () + aOutputs[i].filedown + "." + argv[0]).c_str (), file);
      file << title;

      { std::stringstream header;
      writeHeader (argv[0], aOutputs[i].title, header, false);

      writer.printStart (file, header.str ()); }

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
   createFile ((opt.getDirOutput () + "Movies-Lang.html." + argv[0]).c_str (), file);
   title = htmlData[0].target;
   pos = 0;
   while ((pos = title.find ("%1", pos)) != std::string::npos)
      title.replace (pos, 2, _("Language"));
   file << title;

   writeHeader (argv[0], "[n-d-y-g-m]", file);

   file << "<div class=\"header\">|";
   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l) {
      file << " <a href=\"#" << l->first << "\"><img src=\"images/" << l->first
	   << ".png\" alt=\"" << l->first << " \"> "
	   << _(l->second.getInternational ().c_str ()) << "</a> |";
   }
   file << "</div>";

   MovieWriter langWriter ("%l|%n|%d||%y|%g|%t", movieGenres);
   std::sort (movies.begin (), movies.end (), &Movie::compByName);

   writer.printStart (file, "");

   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l) {
      file << "<tr><td colspan=\"6\"><div class=\"header\"><a name=\"" << l->first << "\">\n<br><h2>"
	   << _(l->second.getInternational ().c_str ()) << "</div></h2></td></tr>";

      file << "<tr><td>";
      writeHeader (argv[0], "[=]![n]![d]![y]![g]![m]", file);
      file << "</td></tr>";

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
   TRACE9 ("CDWriter::readHeaderFile (const char*) - " << file << " - "
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

   Language::init (false);

   return appl.run ();
}
