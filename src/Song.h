#ifndef SONG_H
#define SONG_H

//$Id: Song.h,v 1.6 2004/11/17 17:33:45 markus Exp $

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


#include <glibmm/ustring.h>

#include <YGP/ATime.h>
#include <YGP/Handle.h>
#include <YGP/ANumeric.h>

#include <YGP/Entity.h>


/**Class to hold an interpret
 */
class Song : public YGP::Entity {
   friend class CDManager;
   friend class SongList;

 public:
   Song () : id (0){ duration.setMode (YGP::ATime::MODE_MMSS); }
   Song (const Song& other) : id (other.id), name (other.name),
      track (other.track), duration (other.duration), genre (other.genre) { }
   virtual ~Song () { }

 private:
   unsigned long int id;
   Glib::ustring     name;
   YGP::ANumeric     track;
   YGP::ATime        duration;
   unsigned long int genre;

   //Prohibited manager functions
   const Song& operator= (const Song& other);
};
defineHndl(Song);

#endif
