//$Id: CDAppl.cpp,v 1.17 2008/11/20 10:46:02 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Application
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.17 $
//AUTHOR      : Markus Schwab
//CREATED     : 22.12.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006

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

#include <glibmm/convert.h>

#include <YGP/INIFile.h>

#if WITH_MOVIES == 1
#  include "Movie.h"
#endif
#include "CDManager.h"

#include "CDAppl.h"


const YGP::IVIOApplication::longOptions CDAppl::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "user", 'u' },
   { "password", 'p' },
   { "file", 'f' },
   { "version", 'V' },
   { NULL, '\0' } };


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDAppl::~CDAppl () {
}


//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void CDAppl::showHelp () const {
   std::cout << _("Utility to manage CDs\n\nUsage:") << PACKAGE
             << _(" [OPTIONS]\n\n")
             << "  -u, --user ....... " << _("[USER] User for database login\n")
             << "  -p, --password ... " << _("[PWD] Password for database login\n")
             << "  -f, --file ....... " << _("[FILE] Use file as INI file\n")
             << "  -V, --version .... " << _("Output version information and exit\n")
             << "  -h, -?, --help ... " << _("Displays this help and exit\n\n")
             << _("The INI file can have the following entries:\n\n")
             <<  ("  [Database]\n"
		  "  User=user\n"
		  "  Password=pwd\n\n"
		  "  [Export]\n"
		  "  MovieHead=Movies.head\n"
		  "  MovieFoot=Movies.foot\n"
		  "  RecordHead=Records.head\n"
		  "  RecordFoot=Records.foot\n"
		  "  OutputDir=/var/www/cds/\n"
#if WITH_MOVIES == 1
		  "  \n[Movies]\n"
		  "  Language=de\n"
#endif
);
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require :
///     option not '\0´'
//-----------------------------------------------------------------------------
bool CDAppl::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
   case 'u':
      if (checkOptionValue ())
	 options.user = getOptionValue ();
      else
         std::cerr << PACKAGE << _("-warning: No user specified! Ignoring option `u'\n");
      break;

   case 'p':
      if (checkOptionValue ())
	 options.password = getOptionValue ();
      else
         std::cerr << PACKAGE << _("-warning: No password specified! Ignoring option `p'\n");
      break;

   case 'f': {
      const char* pFile (getOptionValue ());
      if (pFile)
         readINIFile (pFile);
      else
         std::cerr << PACKAGE << _("-warning: No file specified! Ignoring option `f'\n");
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
/// Reads the options of the INI-file
/// \param pFile: Pointer to filename
/// \param Requieres : pFile not NULL
//-----------------------------------------------------------------------------
void CDAppl::readINIFile (const char* pFile) {
   try {
      INIFILE (pFile);
      // DB
      INISECTION (Database);
      INIATTR2 (Database, std::string, options.user, User);
      INIATTR2 (Database, std::string, options.password, Password);

      // Export-otions
      INIOBJ (options, Export);

#if WITH_MOVIES == 1
      // Language in which to show the movies
      INISECTION (Movies);
      INIATTR2 (Movies, std::string, Movie::currLang, Language);
#endif

      INIFILE_READ ();
   }
   catch (YGP::FileError&) { }
   catch (std::exception& error) {
      std::string err ("-warning: Error reading INI-file `%1'! %2\n");
      err.replace (err.find ("%1"), 2, pFile);
      err.replace (err.find ("%2"), 2, error.what ());
      std::cerr << name () << err;
   }
   options.pINIFile = pFile;
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDAppl::perform (int, const char**) {
   try {
      if (options.password.size ())
	 options.password = Glib::locale_to_utf8 (options.password);
   }
   catch (Glib::ConvertError& e) {
      options.password.clear ();
      std::cerr << PACKAGE << _("-warning: Can't convert password to UTF-8! Ignoring ...\n");
   }

   try {
      if (options.user.size ())
	 options.user = Glib::locale_to_utf8 (options.getUser ());
   }
   catch (Glib::ConvertError& e) {
      options.user.clear ();
      std::cerr << PACKAGE << _("-warning: Can't convert username to UTF-8! Ignoring ...\n");
   }

   CDManager win (options);
   Gtk::Main::run (win);
   return 0;
}

//-----------------------------------------------------------------------------
/// Returns a short description of the program (not the help!)
/// \returns const char*: Pointer to a short description
//-----------------------------------------------------------------------------
const char* CDAppl::description () const {
   static std::string version =
      (PACKAGE " V" VERSION " - "
       + std::string (_("Compiled on"))
       + std::string (" " __DATE__ " - " __TIME__ "\n\n")
       + std::string (_("Copyright (C) 2004 - 2008 Markus Schwab; e-mail: g17m0@lycos.com"
			"\nDistributed under the terms of the GNU General "
			"Public License")));
   return version.c_str ();
 }


//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc: Number of parameters
/// \param argv: Array with pointer to parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main (int argc, char* argv[]) {
   YGP::IVIOApplication::initI18n (PACKAGE, LOCALEDIR);

   Gtk::Main gtk (argc, argv);
   CDAppl appl (argc, const_cast<const char**> (argv));
   return appl.run ();
}
