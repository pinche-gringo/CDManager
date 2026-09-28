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

#include <memory>

#include <glibmm/ustring.h>

#include <YGP/ANumeric.h>
#include <YGP/ATime.h>

#include <YGP/Entity.h>

using HEntity = std::shared_ptr<YGP::Entity>;

/**Class to hold an interpret
 */
class Song : public YGP::Entity {
  public:
    Song() { duration.setMode(YGP::ATime::MODE_MMSS); }
    Song(const Song& other) : id(other.id), name(other.name), track(other.track), duration(other.duration), genre(other.genre) {}
    ~Song() override = default;

    Song& operator=(const Song& other) {
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

    [[nodiscard]] unsigned long int getId() const { return id; }
    [[nodiscard]] const Glib::ustring& getName() const { return name; }
    [[nodiscard]] const YGP::ANumeric& getTrack() const { return track; }
    [[nodiscard]] const YGP::ATime& getDuration() const { return duration; }
    [[nodiscard]] unsigned int getGenre() const { return genre; }

    void setId(const unsigned long int value) { id = value; }
    void setName(const Glib::ustring& value) { name = value; }
    void setTrack(const YGP::ANumeric& value) { track = value; }
    void setTrack(const std::string& value) { track = value; }
    void setDuration(const YGP::ATime& value) { duration = value; }
    void setDuration(const std::string& value) { duration = value; }
    void setGenre(const unsigned int value) { genre = value; }

  private:
    unsigned long int id{};
    Glib::ustring name;
    YGP::ANumeric track;
    YGP::ATime duration;
    unsigned long int genre{};
};
using HSong = std::shared_ptr<Song>;

#endif
