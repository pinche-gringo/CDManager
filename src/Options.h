#ifndef OPTIONS_H
#define OPTIONS_H

//$Id: Options.h,v 1.1 2004/12/07 03:33:57 markus Rel $

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


#include <string>
#include <vector>

#include <cdmgr-cfg.h>


class Options {
 public:
   Options::Options () : header ("movie.header") , footer ("movie.footer"),
      dirOutput ("/var/www/cds/") { }
   virtual ~Options () { }

   std::string getHeader () const { return header; }
   std::string getFooter () const { return footer; }
   std::string getDirOutput () const { return dirOutput; }

   void setHeader (const std::string& value) { header = value; }
   void setFooter (const std::string& value) { footer = value; }
   void setDirOutput (const std::string& value) { dirOutput = value; }

 private:
   Options (const Options& other);
   const Options& operator= (const Options& other);

   std::string header;
   std::string footer;
   std::string dirOutput;
};


#endif
