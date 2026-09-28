#ifndef FILM_H
#define FILM_H

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

#include <map>
#include <string>

#include <memory>

#include <glibmm/ustring.h>

#include <XGP/XAttribute.h>
#include <YGP/AYear.h>
#include <YGP/Entity.h>

#include "CDType.h"
#include "EntityAttributes.h"
#include "Language.h"

class Film;
using HFilm = std::shared_ptr<Film>;

/**Class to hold a film
 */
class Film : public YGP::Entity {
  public:
    Film();
    Film(const Film& other);
    ~Film() override = default;

    Film& operator=(const Film& other);

    [[nodiscard]] unsigned long int getId() const { return id; }
    [[nodiscard]] const Glib::ustring& getName() { return getName(currLang); }
    [[nodiscard]] const Glib::ustring& getName(const std::string& lang);
    [[nodiscard]] const YGP::AYear& getYear() const { return year; }
    [[nodiscard]] unsigned int getGenre() const { return genre; }
    [[nodiscard]] int getType() const { return type; }
    [[nodiscard]] const std::string& getLanguage() const { return lang; }
    [[nodiscard]] const std::string& getTitles() const { return titles; }
    [[nodiscard]] const Glib::ustring& getDescription() const { return summary; }
    [[nodiscard]] const std::string& getImage() const { return icon; }

    void setId(unsigned long int value) { id = value; }
    void setName(const Glib::ustring& value);
    void setName(const Glib::ustring& value, const std::string& lang);
    [[nodiscard]] const std::map<std::string, Glib::ustring>& getNames() { return name; }
    void setYear(const YGP::AYear& value) { year = value; }
    void setYear(const std::string& value) { year = value; }
    void setGenre(unsigned int value) { genre = value; }
    void setType(int value) {
        CDType::getInstance()[value];
        type = value;
    }
    void setType(const std::string& value) { type = CDType::getInstance()[value]; }
    void setLanguage(const std::string& value) { lang = value; }
    void setTitles(const std::string& value) { titles = value; }
    void setDescription(const Glib::ustring& value) { summary = value; }
    void setImage(const std::string& value) { icon = value; }

    [[nodiscard]] static Glib::ustring removeIgnored(const Glib::ustring& name);
    [[nodiscard]] static bool compByName(const HFilm& a, const HFilm& b) pre(a) pre(b);
    [[nodiscard]] static bool compByYear(const HFilm& a, const HFilm& b) pre(a) pre(b);
    [[nodiscard]] static bool compByGenre(const HFilm& a, const HFilm& b) pre(a) pre(b);
    [[nodiscard]] static bool compByMedia(const HFilm& a, const HFilm& b) pre(a) pre(b);

    static std::string currLang;

  private:
    unsigned long int id{};
    [[=Attrib{"Name"}]] std::map<std::string, Glib::ustring> name;
    [[=Attrib{"Made"}]] YGP::AYear year;
    [[=Attrib{"Genre"}]] unsigned int genre{};
    [[=Attrib{"Media"}]] int type{};
    [[=Attrib{"Lang"}]] std::string lang;
    [[=Attrib{"Subtitles"}]] std::string titles;
    [[=Attrib{"Description"}]] Glib::ustring summary;
    std::string icon;
};

#endif
