#ifndef CDWRITER_H
#define CDWRITER_H

//$Id: CDWriter.h,v 1.1 2005/01/10 02:10:42 markus Exp $

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

#include "Options.h"

#include <YGP/IVIOAppl.h>


/**Application to write records and movies to HTML pages.
 *
 *The data to write to is read from stdin.
*/
class CDWriter : public YGP::IVIOApplication {
 public:
   CDWriter (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo) { }
   ~CDWriter ();

 protected:
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool        shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE_NAME; }
   virtual const char* description () const;

   // Help-handling
   virtual void showHelp () const;

   static void createFile (const char* name, std::ofstream& file) throw (Glib::ustring);
   static bool readHeaderFile (const char* file, std::string& target,
			       const Glib::ustring& title);

 private:
   // Prohobited manager functions
   CDWriter ();
   CDWriter (const CDWriter&);
   const CDWriter& operator= (const CDWriter&);

   static const longOptions lo[];

   Options opt;
};

#endif
