//$Id: CDAppl.cpp,v 1.2 2004/12/24 04:09:08 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : <FILLIN>
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
//AUTHOR      : Markus Schwab
//CREATED     : 22.12.2004
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

#include <glibmm/convert.h>

#include <YGP/INIFile.h>

#include "CDManager.h"

#include "CDAppl.h"


const YGP::IVIOApplication::longOptions CDAppl::lo[] = {
   { IVIOAPPL_HELP_OPTION },
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
   std::cout << _("Utility to manage CDs: ") << PACKAGE_NAME
             << _(" [OPTIONS]\n\n")
             << "  -f, --file ......... " << _("[FILE] Use file as INI file\n")
             << "  -V, --version ...... " << _("Output version information and exit\n")
             << "  -h, -?, --help ..... " << _("Displays this help and exit\n\n")
             << _("The INI file can have the following entries:\n\n")
             <<  "  [Export]\n"
                 "  MovieHead=Movies.head\n"
                 "  MovieFoot=Movies.foot\n"
                 "  RecordHead=Records.head\n"
                 "  RecordFoot=Records.foot\n"
                 "  OutputDir=/var/www/cds/\n";
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require :
///     option not '\0�'
//-----------------------------------------------------------------------------
bool CDAppl::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
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
      INIOBJ (options, Export);

      INIFILE_READ ();
      options.pINIFile = pFile;
   }
   catch (std::string& error) {
      Glib::ustring err ("-warning: Error reading INI-file `%1'");
      err.replace (err.find ("%1"), 2, pFile);
      std::cerr << PACKAGE << err << '\n';
   }
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDAppl::perform (int, const char**) {
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
       + std::string (_("Author: Markus Schwab; e-mail: g17m0@lycos.com"
			"\nDistributed under the terms of the GNU General "
			"Public License")));
   return version.c_str ();
 }
