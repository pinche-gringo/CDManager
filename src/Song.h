#ifndef SONG_H
#define SONG_H

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

#include <YGP/ATime.h>
#include <YGP/ANumeric.h>

#include <YGP/Entity.h>


typedef boost::shared_ptr<YGP::Entity> HEntity;


/**Class to hold an interpret
 */
class Song : public YGP::Entity {
 public:
   Song () : id (0), genre (0) { duration.setMode (YGP::ATime::MODE_MMSS); }
   Song (const Song& other) : id (other.id), name (other.name),
      track (other.track), duration (other.duration), genre (other.genre) { }
   virtual ~Song () { }

   Song& operator= (const Song& other) {
      if (this != &other) {
	 if (!id)
	    id = other.id;
	 name = other.name;
	 track = other.track;
	 duration = other.duration;
	 genre = other.genre;
      }
      return *this;
   }

   unsigned long int    getId () const { return id; }
   const Glib::ustring& getName () const { return name; }
   const YGP::ANumeric& getTrack () const { return track; }
   const YGP::ATime&    getDuration () const { return duration; }
   unsigned int         getGenre () const { return genre; }

   void setId       (const unsigned long int value) { id = value; }
   void setName     (const Glib::ustring& value) { name = value; }
   void setTrack    (const YGP::ANumeric& value) { track = value; }
   void setTrack    (const std::string& value) throw (std::invalid_argument) { track = value; }
   void setDuration (const YGP::ATime& value) { duration = value; }
   void setDuration (const std::string& value) throw (std::invalid_argument) { duration = value; }
   void setGenre    (const unsigned int value) { genre = value; }

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::ANumeric     track;
   YGP::ATime        duration;
   unsigned long int genre;
};
typedef boost::shared_ptr<Song> HSong;

#endif
