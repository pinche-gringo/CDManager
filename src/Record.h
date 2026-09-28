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

#include <memory>

#include <glibmm/ustring.h>

#include <YGP/AYear.h>
#include <YGP/Entity.h>

#include "EntityAttributes.h"

class Record;
using HRecord = std::shared_ptr<Record>;

/**Class to hold a record
 */
class Record : public YGP::Entity {
  public:
    Record();
    Record(const Record& other);
    ~Record() override = default;

    Record& operator=(const Record& other);

    [[nodiscard]] unsigned long int getId() const { return id; }
    [[nodiscard]] const Glib::ustring& getName() const { return name; }
    [[nodiscard]] const YGP::AYear& getYear() const { return year; }
    [[nodiscard]] unsigned int getGenre() const { return genre; }

    void setId(const unsigned long int value) { id = value; }
    void setName(const Glib::ustring& value) { name = value; }
    void setYear(const YGP::AYear& value) { year = value; }
    void setYear(const std::string& value) { year = value; }
    void setGenre(const unsigned int value) { genre = value; }

    [[nodiscard]] bool needsLoading() const { return loadSongs; }
    void setSongsLoaded() { loadSongs = false; }

    [[nodiscard]] static Glib::ustring removeIgnored(const Glib::ustring& name);
    [[nodiscard]] static bool compByName(const HRecord& a, const HRecord& b) pre(a) pre(b);
    [[nodiscard]] static bool compByYear(const HRecord& a, const HRecord& b) pre(a) pre(b);
    [[nodiscard]] static bool compByGenre(const HRecord& a, const HRecord& b) pre(a) pre(b);

  private:
    unsigned long int id{};
    [[=Attrib{"Name"}]] Glib::ustring name;
    [[=Attrib{"Made"}]] YGP::AYear year;
    [[=Attrib{"Genre"}]] unsigned int genre{};
    bool loadSongs{true};
};

#endif
