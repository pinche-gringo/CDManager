#ifndef OPTIONS_H
#define OPTIONS_H

//$Id: Options.h,v 1.3 2005/01/13 19:19:32 markus Rel $

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

#include <YGP/Entity.h>


class Options : public YGP::Entity {
   friend class Settings;

 public:
   Options::Options ();
   virtual ~Options ();

   const std::string& Options::getMHeader () const { return mHeader; }
   const std::string& getMFooter () const { return mFooter; }
   const std::string& getRHeader () const { return rHeader; }
   const std::string& getRFooter () const { return rFooter; }
   const std::string& getDirOutput () const { return dirOutput; }

   void setMHeader (const std::string& value) { mHeader = value; }
   void setMFooter (const std::string& value) { mFooter = value; }
   void setRHeader (const std::string& value) { rHeader = value; }
   void setRFooter (const std::string& value) { rFooter = value; }
   void setDirOutput (const std::string& value) { dirOutput = value; }

   const char* pINIFile;

 private:
   Options (const Options& other);
   const Options& operator= (const Options& other);

   std::string mHeader;             // %attrib%; MovieHead;       "Movies.head"
   std::string mFooter;             // %attrib%; MovieFoot;       "Movies.foot"
   std::string rHeader;             // %attrib%; RecordHead;     "Records.head"
   std::string rFooter;             // %attrib%; RecordFoot;     "Records.foot"
   std::string dirOutput;           // %attrib%; OutputDir;     "/var/www/cds/"
};

#endif
