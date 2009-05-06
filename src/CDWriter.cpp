//$Id: CDWriter.cpp,v 1.27 2009/05/06 09:13:06 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : CDWriter
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.27 $
//AUTHOR      : Markus Schwab
//CREATED     : 07.01.2005
//COPYRIGHT   : Copyright (C) 2005 - 2007, 2009

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

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <glibmm/ustring.h>
#include <glibmm/convert.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ADate.h>
#include <YGP/ATStamp.h>
#include <YGP/Relation.h>
#include <YGP/Tokenize.h>
#include <YGP/Exception.h>

#include "DB.h"
#include "Words.h"
#include "Genres.h"
#include "Writer.h"
#include "Language.h"

#if WITH_MOVIES == 1
#  include "Movie.h"
#  include "Director.h"
#endif

#if WITH_RECORDS == 1
#  include "Record.h"
#  include "Interpret.h"
#endif

#include "CDWriter.h"
#include "Options.meta"


const YGP::IVIOApplication::longOptions CDWriter::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "version", 'V' },
#if WITH_RECORDS == 1
   { "recHeader", 'r' },
   { "recFooter", 'R' },
#endif
#if WITH_MOVIES == 1
   { "movieHeader", 'm' },
   { "movieFooter", 'M' },
#endif
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
   std::cout << _("Utitily to write HTML documents from data received\n\nUsage: ") << name ()
             << _(" [OPTIONS] [LANGUAGE-ID] [MEMORY-ID]\n\n")
	     << "  -d, --outputDir ..... " << _("Directory to export data to\n")
#if WITH_RECORDS == 1
	     << "  -r, --recHeader ..... " << _("File to use as header for records\n")
	     << "  -R, --recFooter ..... " << _("File to use as footer for records\n")
#endif
#if WITH_MOVIES == 1
	     << "  -m, --movieHeader ... " << _("File to use as header for movies\n")
	     << "  -M, --movieFooter ... " << _("File to use as footer for movies\n")
#endif
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
         std::cerr << name () << _("-warning: No directory specified! Ignoring option `d'\n");
      break; }

#if WITH_MOVIES == 1
   case 'm':
   case 'M': {
      const char* pFile (getOptionValue ());
      if (pFile)
	 (option == 'm') ? opt.setMHeader (pFile) : opt.setMFooter (pFile);
      else {
	 Glib::ustring e (_("-warning: No file specified! Ignoring option `%1'\n"));
	 e.replace (e.find ("%1"), 2, 1, option);
         std::cerr << name () << e;
      }
      break; }

#endif
#if WITH_RECORDS == 1
   case 'R':
   case 'r': {
      const char* pFile (getOptionValue ());
      if (pFile)
	 (option == 'r') ?  opt.setRHeader (pFile) : opt.setRFooter (pFile);
      else {
	 Glib::ustring e (_("-warning: No file specified! Ignoring option `%1'\n"));
	 e.replace (e.find ("%1"), 2, 1, option);
         std::cerr << name () << e;
      }
      break; }
#endif

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
/// \param lead: String with which to start filenames
/// \remarks Other characters are just copied
//-----------------------------------------------------------------------------
void CDWriter::writeHeader (const char* lang, const char* format,
			    std::ostream& stream, bool upSorted,
			    const char* lead) {
   TRACE9 ("CDWriter::writeHeader (2x const char*, std::ostream&, bool, const char*) - "
	   << lang << "; " << format << "; " << lead);
   Check3 (lang); Check3 (format); Check3 (lead);

   static const char formats[] = "adnygml-![]=";
   static const char* docs[] = { "", "", "-Name", "-Year", "-Genre", "-Media",
			   "-Lang", NULL, NULL, NULL, NULL, NULL };
   static const std::string titles[] =
      { _("Interpret"), _("Director"), _("Name"), _("Year"),_("Genre"),
	_("Media"), _("Language(s)"), " | ", "</td><td>",
	"<div class=\"header\">", "</div>", "&nbsp;" };
   Check3 ((sizeof (docs) / sizeof (*docs)) == strlen (formats));
   Check3 ((sizeof (titles) / sizeof (*titles)) == strlen (formats));

   while (*format) {
      char* posFormat (strchr (formats, *format));
      if (posFormat) {
	 unsigned int pos (strchr (formats, *format++) - formats);
	 Check3 (titles[pos].size ());

	 if (docs[pos]) {
	    stream << "<a href=\"" << lead << docs[pos];
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
   if (argc != 2) {
      std::cerr << name () << _("-error: Need language id and memory-key as parameters\n"
				"(This program is designed to be called by the CDManager-application)\n");
      return - 1;
   }

   Genres movieGenres, recGenres;

   try {
      if (!atoi (argv[1]))
	 throw std::invalid_argument (_("Invalid memory-key (0)!"));

      Words::access (atoi (argv[1]));
      TRACE9 ("Words: " << Words::getMemoryKey () << ": " << Words::cArticles () << '/' << Words::cNames ());

      Genres::loadFromFile (DATADIR "Genres.dat", recGenres, movieGenres, *argv);
   }
   catch (std::invalid_argument& e) {
      std::string msg (_("-error: Can't access reserved words!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, e.what ());
      std::cerr << name () << msg << '\n';
      return -2;
   }
   catch (std::exception& e) {
      std::string msg (_("Can't read datafile containing the genres!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, e.what ());
      std::cerr << name () << msg << '\n';
      return -3;
   }

#if WITH_MOVIES == 1
   Movie::currLang = *argv;
   Glib::ustring transTitleMovie (_("Movies (by %1)"));
#endif
#if WITH_RECORDS == 1
   Glib::ustring transTitleRecord (_("Records (by %1)"));
#endif

   struct {
      std::string    name;
      Glib::ustring& source;
      std::string    target;
   } htmlData[] = {
#if WITH_MOVIES == 1
      { opt.getMHeader (), transTitleMovie },
      { opt.getMFooter (), transTitleMovie }
#  if WITH_RECORDS == 1
      ,
#  endif
#endif
#if WITH_RECORDS == 1
      { opt.getRHeader (), transTitleRecord },
      { opt.getRFooter (), transTitleRecord }
#endif
 };

   for (unsigned int i (0); i < (sizeof (htmlData) / sizeof (*htmlData)); ++i) {
      if (htmlData[i].name.size ()
	  && (htmlData[i].name[0] != YGP::File::DIRSEPARATOR))
	 htmlData[i].name = DATADIR + htmlData[i].name;
      if (htmlData[i].target.size ()
	  && (htmlData[i].target[0] != YGP::File::DIRSEPARATOR))
	 htmlData[i].target = DATADIR + htmlData[i].target;

      if (!readHeaderFile (htmlData[i].name.c_str (), argv[0], htmlData[i].target,
			   htmlData[i].source)) {
	 std::string error ( (_("Error reading header file `%1'!\n\nReason: %2")));
	 error.replace (error.find ("%1"), 2, htmlData[i].name);
	 error.replace (error.find ("%2"), 2, strerror (errno));

	 TRACE1 ("CDWriter::perform (int, const char**) - Error reading HTML-header/footer:\n\t" << msg);
	 std::cerr << name () << ": " << error << '\n';
	 return -4;
      }
   } // end-for

   size_t pos (0);
#if WITH_MOVIES == 1
   std::ofstream fileMovie;
   createFile (opt.getDirOutput () + "Movies.html", argv[0], fileMovie);

   // Writing the title for movies
   std::string titleMovie (htmlData[0].target);
   pos = 0;
   while ((pos = titleMovie.find ("%1", pos)) != std::string::npos)
      titleMovie.replace (pos, 2, _("Director"));
   fileMovie << titleMovie;

   writeHeader (argv[0], "[d-n-y-g-m-l]", fileMovie);

   MovieWriter movieWriter ("%n|%y|%g|%t|%l", movieGenres);
   movieWriter.printStart (fileMovie, "");

   HMovie movie;
   HDirector director;
   std::vector<HMovie> movies;
   std::vector<HDirector> directors;
   YGP::Relation1_N<HDirector, HMovie> relMovies ("movies");
   std::string usedLanguages;
#endif

#if WITH_RECORDS == 1
   std::ofstream fileRec;
   createFile (opt.getDirOutput () + "Records.html", argv[0], fileRec);

   // Writing the title for records
   std::string titleRec (htmlData[WITH_MOVIES << 1].target);
   pos = 0;
   while ((pos = titleRec.find ("%1", pos)) != std::string::npos)
      titleRec.replace (pos, 2, _("Interpret"));
   fileRec << titleRec;

   writeHeader (argv[0], "[a-n-y-g]", fileRec, true, "Records");

   RecordWriter recWriter ("%n|%y|%g", recGenres);
   recWriter.printStart (fileRec, "");

   HRecord record;
   HInterpret artist;
   std::vector<HRecord> records;
   std::vector<HInterpret> artists;
   YGP::Relation1_N<HInterpret, HRecord> relRecords ("records");
#endif

   // Read input from stdin; both movies and records can be handled
   char type;
   std::cin >> type;
   while (!std::cin.eof ()) {
      try {
	 TRACE4 ("CDWriter::perform (int, char**) - Type: " << type);
	 switch (type) {
#if WITH_MOVIES == 1
	 case 'D':
	    director.define ();
	    std::cin >> *director;
	    TRACE9 ("CDWriter::perform (int, char**) - Director: " << director->getName ());
	    directors.push_back (director);
	    break;

	 case 'M': {
	    Check3 (director.isDefined ());
	    movie.define ();
	    std::cin >> *movie;
	    TRACE9 ("CDWriter::perform (int, char**) - Movie: " << movie->getName ());
	    if (!relMovies.isRelated (director))
	       movieWriter.writeDirector (director, fileMovie);

	    relMovies.relate (director, movie);
	    movieWriter.writeMovie (movie, director, fileMovie);
	    movies.push_back (movie);

	    YGP::Tokenize langs (movie->getLanguage ());
	    while (langs.getNextNode (',').size ()) {
	       if (usedLanguages.find (langs.getActNode ()) == std::string::npos) {
		  usedLanguages += ',';
		  usedLanguages += langs.getActNode ();
	       }
	    }
	    break; }
#endif

#if WITH_RECORDS == 1
	 case 'I':
	    artist.define ();
	    std::cin >> *artist;
	    TRACE9 ("CDWriter::perform (int, char**) - Artist: " << artist->getName ());
	    artists.push_back (artist);
	    break;

	 case 'R':
	    Check3 (artist.isDefined ());
	    record.define ();
	    std::cin >> *record;
	    TRACE9 ("CDWriter::perform (int, char**) - Record: " << record->getName ());
	    if (!relRecords.isRelated (artist))
	       recWriter.writeInterpret (artist, fileRec);

	    relRecords.relate (artist, record);
	    recWriter.writeRecord (record, artist, fileRec);
	    records.push_back (record);
	    break;
#endif

	 default:
	    Check3 (0);
	    return -1;
	 }
      }
      catch (std::exception& error) {
	 const char* types ("DMIR");
	 const char* what[] = { N_("Director"), N_("Movie"), N_("Interpret"), N_("Record") };
	 Check9 ((sizeof (types) / sizeof (*types)) == (sizeof (what) / sizeof (*what)));
	 const char* i (strchr (types, type));
	 Glib::ustring entity (_(i ? what[i - types] : N_("unknown entity")));

	 Glib::ustring msg ( _("-error: Can't read %1: %2"));
	 msg.replace (msg.find ("%1"), 2, entity);
	 msg.replace (msg.find ("%2"), 2, error.what ());
	 std::cerr << name () << msg << '\n';
	 break;
      }
      catch (...) {
	 std::cerr << name () << _("-error: Unknown error!\n");
      }

      std::cin >> type;
   } // end-while not eof

#if WITH_MOVIES == 1
   movieWriter.printEnd (fileMovie);
   fileMovie << htmlData[1].target;
   fileMovie.close ();

   // Write reverse file
   createFile (opt.getDirOutput () + "Moviesdown.html", argv[0], fileMovie);
   fileMovie << titleMovie;
   writeHeader (argv[0], "[d-n-y-g-m-l]", fileMovie, false);

   movieWriter.printStart (fileMovie, "");
   for (std::vector<HDirector>::reverse_iterator i (directors.rbegin ());
	i != directors.rend (); ++i)
      if (relMovies.isRelated (*i)) {
	 movieWriter.writeDirector (*i, fileMovie);

	 const std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	 Check3 (dirMovies.size ());
	 for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
	      m != dirMovies.end (); ++m)
	    movieWriter.writeMovie (*m, *i, fileMovie);
      }
   movieWriter.printEnd (fileMovie);
   fileMovie << htmlData[1].target;
   fileMovie.close ();

   typedef bool (*PFNCMPMOVIE) (const HMovie&, const HMovie&);
#endif

#if WITH_RECORDS == 1
   recWriter.printEnd (fileRec);
   fileRec.flush ();
   fileRec << htmlData[(WITH_MOVIES << 1) + 1].target;
   fileRec.close ();

   // Write reverse file
   createFile (opt.getDirOutput () + "Recordsdown.html", argv[0], fileRec);
   fileRec << titleRec;
   writeHeader (argv[0], "[a-n-y-g]", fileRec, false, "Records");

   recWriter.printStart (fileRec, "");
   for (std::vector<HInterpret>::reverse_iterator i (artists.rbegin ());
	i != artists.rend (); ++i)
      if (relRecords.isRelated (*i)) {
	 recWriter.writeInterpret (*i, fileRec);

	 const std::vector<HRecord>& dirRecords (relRecords.getObjects (*i));
	 Check3 (dirRecords.size ());
	 for (std::vector<HRecord>::const_iterator m (dirRecords.begin ());
	      m != dirRecords.end (); ++m)
	    recWriter.writeRecord (*m, *i, fileRec);
      }
   recWriter.printEnd (fileRec);
   fileRec << htmlData[(WITH_MOVIES << 1) + 1].target;
   fileRec.close ();

   typedef bool (*PFNCMPRECORD) (const HRecord&, const HRecord&);
#endif

   struct {
      const char* title;
      const char* file;
      const char* filedown;
      const char* format;
      const char* sorted;
      void*  fnCompare;
      const char* lead;
      unsigned int type;
   } aOutputs[] = {
#if WITH_MOVIES == 1
	// Entries for movies
      { "[n]|[d]|[y]|[g]|[m]|[l]", "Movies-Name.html", "Movies-Namedown.html",
	"%n|%d|%y|%g|%t|%l", N_("Name"), (void*)&Movie::compByName, "Movies", 0 },
      { "[y]|[n]|[d]|[g]|[m]|[l]", "Movies-Year.html", "Movies-Yeardown.html",
	"%y|%n|%d|%g|%t|%l", N_("Year"), (void*)&Movie::compByYear, "Movies", 0 },
      { "[g]|[n]|[d]|[y]|[m]|[l]", "Movies-Genre.html", "Movies-Genredown.html",
	"%g|%n|%d|%y|%t|%l", N_("Genre"), (void*)&Movie::compByGenre, "Movies", 0 },
      { "[m]|[n]|[d]|[y]|[g]|[l]", "Movies-Media.html", "Movies-Mediadown.html",
	"%t|%n|%d|%y|%g|%l", N_("Media"), (void*)&Movie::compByMedia, "Movies", 0 }
#  if WITH_RECORDS == 1
      ,
#  endif
#endif
#if WITH_RECORDS == 1
      // Entries for records
      { "[n]|[a]|[y]|[g]", "Records-Name.html", "Records-Namedown.html",
	"%n|%d|%y|%g", N_("Name"), (void*)&Record::compByName, "Records", 1 },
      { "[y]|[n]|[a]|[g]", "Records-Year.html", "Records-Yeardown.html",
	"%y|%n|%d|%g", N_("Year"), (void*)&Record::compByYear, "Records", 1 },
      { "[g]|[n]|[a]|[y]", "Records-Genre.html", "Records-Genredown.html",
	"%g|%n|%d|%y", N_("Genre"), (void*)&Record::compByGenre, "Records", 1 }
#endif
 };

   std::ofstream fileOut;
   std::string strTitle;
   // This combines writing movies and records
   for (unsigned int i (0); i < (sizeof (aOutputs) / sizeof (*aOutputs)); ++i) {
      createFile (opt.getDirOutput () + aOutputs[i].file, argv[0], fileOut);
      strTitle = htmlData[aOutputs[i].type << 1].target;
      pos = 0;
      while ((pos = strTitle.find ("%1", pos)) != std::string::npos)
	 strTitle.replace (pos, 2, _(aOutputs[i].sorted));

      fileOut << strTitle;

      std::stringstream header;
      writeHeader (argv[0], aOutputs[i].title, header, true, aOutputs[i].lead);

#if WITH_RECORDS == 1
      if (aOutputs[i].type) {
	 std::sort (records.begin (), records.end (), (PFNCMPRECORD)aOutputs[i].fnCompare);

	 RecordWriter writer (aOutputs[i].format, recGenres);
	 writer.printStart (fileOut, header.str ());

	 HInterpret interpret;
	 for (std::vector<HRecord>::const_iterator m (records.begin ());
	      m != records.end (); ++m) {
	    interpret = relRecords.getParent (*m); Check3 (interpret.isDefined ());
	    writer.writeRecord (*m, interpret, fileOut);
	 }
	 writer.printEnd (fileOut);
      }
      else
#endif
#if WITH_MOVIES == 1
      {
	 std::sort (movies.begin (), movies.end (), (PFNCMPMOVIE)aOutputs[i].fnCompare);

	 MovieWriter writer (aOutputs[i].format, movieGenres);
	 writer.printStart (fileOut, header.str ());

	 HDirector director;
	 for (std::vector<HMovie>::const_iterator m (movies.begin ());
	      m != movies.end (); ++m) {
	    director = relMovies.getParent (*m); Check3 (director.isDefined ());
	    writer.writeMovie (*m, director, fileOut);
	 }
	 writer.printEnd (fileOut);
      }
#endif

      fileOut << htmlData[(aOutputs[i].type << 1) + 1 ].target;
      fileOut.close ();

      createFile (opt.getDirOutput () + aOutputs[i].filedown, argv[0], fileOut);
      fileOut << strTitle;

      std::stringstream rheader;
      writeHeader (argv[0], aOutputs[i].title, rheader, false, aOutputs[i].lead);

#if WITH_RECORDS == 1
      if (aOutputs[i].type) {
	 RecordWriter writer (aOutputs[i].format, recGenres);
	 writer.printStart (fileOut, rheader.str ());

	 for (std::vector<HRecord>::reverse_iterator m (records.rbegin ());
	      m != records.rend (); ++m) {
	    HInterpret interpret;
	    interpret = relRecords.getParent (*m); Check3 (interpret.isDefined ());
	    writer.writeRecord (*m, interpret, fileOut);
	 }
	 writer.printEnd (fileOut);
      }
      else
#endif
#if WITH_MOVIES == 1
      {
	 MovieWriter writer (aOutputs[i].format, movieGenres);
	 writer.printStart (fileOut, rheader.str ());

	 for (std::vector<HMovie>::reverse_iterator m (movies.rbegin ());
	      m != movies.rend (); ++m) {
	    HDirector director;
	    director = relMovies.getParent (*m); Check3 (director.isDefined ());
	    writer.writeMovie (*m, director, fileOut);
	 }
	 writer.printEnd (fileOut);
      }
#endif

      fileOut << htmlData[(aOutputs[i].type << 1) + 1].target;
      fileOut.close ();
   }

#if WITH_MOVIES == 1
   // Export movies by language
   createFile (opt.getDirOutput () + "Movies-Lang.html", argv[0], fileOut);
   titleMovie = htmlData[0].target;
   pos = 0;
   while ((pos = titleMovie.find ("%1", pos)) != std::string::npos)
      titleMovie.replace (pos, 2, _("Language"));
   fileOut << titleMovie;

   writeHeader (argv[0], "[n-d-y-g-m]", fileOut, false);

   fileOut << "<div class=\"header\">|";
   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l)
      if (usedLanguages.find (l->first) != std::string::npos)
	 fileOut << " <a href=\"#" << l->first << "\"><img src=\"images/" << l->first
		   << ".png\" alt=\"" << l->first << " \">&nbsp;"
		   << l->second.getInternational () << "</a> |";
   fileOut << "</div>";

   MovieWriter langWriter ("%l|%n|%d||%y|%g|%t", movieGenres);
   std::sort (movies.begin (), movies.end (), &Movie::compByName);

   langWriter.printStart (fileOut, "");

   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l) {
      if (usedLanguages.find (l->first) != std::string::npos) {
	 fileOut << "<tr><td colspan=\"6\"><div class=\"header\"><a name=\"" << l->first << "\">\n<br></a><h2>"
		   << l->second.getInternational () << "</h2></div></td></tr>";

	 fileOut << "<tr><td>";
	 writeHeader (argv[0], "[=]![n]![d]![y]![g]![m]", fileOut, false);
	 fileOut << "</td></tr>";

	 for (std::vector<HMovie>::const_iterator m (movies.begin ());
	      m != movies.end (); ++m)
	    if (((*m)->getLanguage ().find (l->first) != std::string::npos)
		|| ((*m)->getTitles ().find (l->first) != std::string::npos)) {
	       HDirector director;
	       director = relMovies.getParent (*m); Check3 (director.isDefined ());
	       langWriter.writeMovie (*m, director, fileOut);
	 }
      }
   }

   langWriter.printEnd (fileOut);
   fileOut << htmlData[1].target;
   fileOut.close ();
#endif

   Words::destroy ();
   return 0;
}

//-----------------------------------------------------------------------------
/// Returns a short description of the program (not the help!)
/// \returns const char*: Pointer to a short description
//-----------------------------------------------------------------------------
const char* CDWriter::description () const {
   static std::string version =
      (name () + std::string ( " V" VERSION " - ")
       + std::string (_("Compiled on"))
       + std::string (" " __DATE__ " - " __TIME__ "\n\n")
       + std::string (_("Copyright (C) 2005 - 2007, 2009 Markus Schwab; e-mail: g17m0@users.sourceforge.net"
			"\nDistributed under the terms of the GNU General "
			"Public License")));
   return version.c_str ();
 }

//-----------------------------------------------------------------------------
/// Creates a file. Errors are reported to std::cerr
/// \param filename: Name of file to create
/// \param lang: Language-id
/// \param file: Created stream
//-----------------------------------------------------------------------------
void CDWriter::createFile (const std::string& filename, const char* lang, std::ofstream& file) {
   TRACE9 ("CDWriter::createFile (const std::string&, const char*, std::ofstream&) - " << name);
   Check1 (filename.size ());
   Check1 (lang);

   std::string utf8file (filename);
   utf8file += '.';
   utf8file += lang;
   utf8file += ".utf8";
   file.open (utf8file.c_str ());
   if (!file) {
      Glib::ustring msg (_("Can't create file `%1'!\n\nReason: %2."));
      msg.replace (msg.find ("%1"), 2, utf8file);
      msg.replace (msg.find ("%2"), 2, strerror (errno));
      std::cerr << name () << _("-error: ") << msg;
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
/// \param lang: ID of language
/// \param target: Variable receiving the input
/// \param title: Value to substitute @TITLE@ with
/// \returns bool: True, if successful
/// \remarks The argument target is overwritten anyway
//-----------------------------------------------------------------------------
bool CDWriter::readHeaderFile (const char* file, const char* lang,
			       std::string& target, const Glib::ustring& title) {
   TRACE9 ("CDWriter::readHeaderFile (const char*) - " << file << " (" << lang
	   << "): " << title);
   Check2 (file);
   Check2 (lang);

   target = file;
   target += '.';
   target += lang;
   std::ifstream input (target.c_str ());
   if (!input) {
      input.clear ();
      input.open (file);
      if (!input)
	 return false;
   }
   target.clear ();

   size_t i (512);
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

   Language::init ();
   return appl.run ();
}
