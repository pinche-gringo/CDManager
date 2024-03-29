#ifndef RECORD_H
#define RECORD_H

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.


#include <boost/shared_ptr.hpp>

#include <glibmm/ustring.h>

#include <YGP/AYear.h>
#include <YGP/Entity.h>


class Record;
typedef boost::shared_ptr<Record> HRecord;


/**Class to hold a record
 */
class Record : public YGP::Entity {
 public:
   Record ();
   Record (const Record& other);
   virtual ~Record ();

   Record& operator= (const Record& other);

   unsigned long int    getId () const {return id; }
   const Glib::ustring& getName () const {return name; }
   const YGP::AYear&    getYear () const { return year; }
   unsigned int         getGenre () const { return genre; }

   void setId       (const unsigned long int value) { id = value; }
   void setName     (const Glib::ustring& value) { name = value; }
   void setYear     (const YGP::AYear& value) { year = value; }
   void setYear     (const std::string& value) { year = value; }
   void setGenre    (const unsigned int value) { genre = value; }

   bool needsLoading () const { return loadSongs; }
   void setSongsLoaded () { loadSongs = false; }

   static Glib::ustring removeIgnored (const Glib::ustring& name);
   static bool compByName (const HRecord& a, const HRecord& b);
   static bool compByYear (const HRecord& a, const HRecord& b);
   static bool compByGenre (const HRecord& a, const HRecord& b);

 private:
   unsigned long int id;             // %attrib%; ; 0
   Glib::ustring     name;           // %attrib%; Name
   YGP::AYear        year;           // %attrib%; Made
   unsigned int      genre;          // %attrib%; Genre; 0
   bool              loadSongs;      // %attrib%; ; true
};

#endif
