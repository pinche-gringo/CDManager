#ifndef CDAPPL_H
#define CDAPPL_H

//$Id: CDAppl.h,v 1.1 2004/12/22 22:51:12 markus Rel $

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


#include <YGP/IVIOAppl.h>

#include "Options.h"


/**VIO-Application part of the CD-manager; cares about reading the INI-file and
 * processing the options
*/
class CDAppl : public YGP::IVIOApplication {
 public:
   CDAppl (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo) { }
   ~CDAppl ();

 protected:
   virtual void readINIFile (const char* pFile);
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool        shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE_NAME; }
   virtual const char* description () const;

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   CDAppl ();
   CDAppl (const CDAppl&);
   const CDAppl& operator= (const CDAppl&);

   Options options;

   static const longOptions lo[];
};

#endif
