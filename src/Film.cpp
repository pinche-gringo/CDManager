// PROJECT     : CDManager
// SUBSYSTEM   : Film
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 29.11.2004
// COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011, 2026

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

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <format>
#include <limits>
#include <stdexcept>
#include <utility>

#include <glibmm/ustring.h>

// #include <XGP/XAttribute.h>  // Needed for specialization of YGP::Attribute for Glib::ustring

#include "Film.h"
#include "Words.h"

// Specialization of YGP::Attribute for the name-map
namespace YGP {

template <> inline bool Attribute<std::map<std::string, Glib::ustring>>::assignFromString(const char* value) const {
    contract_assert(value);
    attr_[Film::currLang] = value;
    return true;
}
template <> inline std::string Attribute<std::map<std::string, Glib::ustring>>::getValue() const {
    return (attr_.find(Film::currLang) == attr_.end()) ? attr_[""] : attr_[Film::currLang];
}
template <> inline std::string Attribute<std::map<std::string, Glib::ustring>>::getFormattedValue() const { return getValue(); }

} // namespace YGP

std::string Film::currLang;

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
Film::Film() { registerAttributes(*this); }

//-----------------------------------------------------------------------------
/// Copyonstructor
//-----------------------------------------------------------------------------
Film::Film(const Film& other)
    : id(other.id), name(other.name), year(other.year), genre(other.genre), size(other.size), lang(other.lang),
      titles(other.titles), summary(other.summary), icon(other.icon) {
    registerAttributes(*this);
}

//-----------------------------------------------------------------------------
/// Assignment operator
/// \param other Object to assign
/// \returns Film& Reference to self
//-----------------------------------------------------------------------------
Film& Film::operator=(const Film& other) {
    if (this != &other) {
        if (!id)
            id = other.id;

        name = other.name;
        year = other.year;
        genre = other.genre;
        size = other.size;
        lang = other.lang;
        titles = other.titles;
        summary = other.summary;
        icon = other.icon;
    }
    return *this;
}

//-----------------------------------------------------------------------------
/// Removes the leading articles of the names.
/// \param name Name to manipulate
/// \returns Glib::ustring Changed name
//-----------------------------------------------------------------------------
Glib::ustring Film::removeIgnored(const Glib::ustring& name) { return Words::removeArticle(name); }

//-----------------------------------------------------------------------------
/// Sorts films by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Film::compByName(const HFilm& a, const HFilm& b) {
    Glib::ustring aname(removeIgnored(a->getName()));
    Glib::ustring bname(removeIgnored(b->getName()));
    const int rc(aname.compare(bname));
    return rc ? (rc < 0) : (a->year < b->year);
}

//-----------------------------------------------------------------------------
/// Sorts films by year.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->year < b->year
//-----------------------------------------------------------------------------
bool Film::compByYear(const HFilm& a, const HFilm& b) {
    long rc(a->year.compare(b->year));
    if (!rc) {
        Glib::ustring aname(removeIgnored(a->getName()));
        Glib::ustring bname(removeIgnored(b->getName()));
        rc = aname.compare(bname);
    }
    return rc < 0;
}

//-----------------------------------------------------------------------------
/// Sorts films by genre.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->genre < b->genre
//-----------------------------------------------------------------------------
bool Film::compByGenre(const HFilm& a, const HFilm& b) {
    int rc(a->genre - b->genre);
    return rc ? (rc < 0) : compByName(a, b);
}

//-----------------------------------------------------------------------------
/// Sorts films by size.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->size < b->size
//-----------------------------------------------------------------------------
bool Film::compBySize(const HFilm& a, const HFilm& b) {
    return (a->size != b->size) ? (a->size < b->size) : compByName(a, b);
}

//-----------------------------------------------------------------------------
/// Formats the passed size in a human readable way (e.g. "700 MB", "1.4 GB")
/// \param size Size in MB (0 for unknown)
/// \returns std::string Formatted size (empty, if the size is unknown)
//-----------------------------------------------------------------------------
std::string Film::formatSize(unsigned int size) {
    if (!size)
        return {};
    if (size < 1000)
        return std::format("{} MB", size);
    if (size < 1000000)
        return std::format("{:.1f} GB", size / 1000.0);
    return std::format("{:.1f} TB", size / 1000000.0);
}

//-----------------------------------------------------------------------------
/// Parses a size (as entered by the user), like "700", "700 MB", "1.4 GB",
/// "1,4G" or "2 TB". Values without unit are in MB.
/// \param value Text to parse; empty for an unknown size
/// \returns unsigned int Size in MB
/// \throws std::invalid_argument If value is not a valid size
//-----------------------------------------------------------------------------
unsigned int Film::parseSize(std::string_view value) {
    const auto isSpace([](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; });
    const auto trim([&isSpace](std::string_view text) {
        while (!text.empty() && isSpace(text.front()))
            text.remove_prefix(1);
        while (!text.empty() && isSpace(text.back()))
            text.remove_suffix(1);
        return text;
    });

    std::string number(trim(value));
    if (number.empty())
        return 0;
    std::ranges::replace(number, ',', '.');

    double size {};
    const char* const last(number.data() + number.size());
    const auto [end, error](std::from_chars(number.data(), last, size, std::chars_format::fixed));
    if ((error != std::errc()) || (size < 0))
        throw std::invalid_argument(std::format("Invalid size `{}'", value));

    std::string unit(trim(std::string_view(end, last)));
    std::ranges::transform(unit, unit.begin(), [](char c) { return std::tolower(static_cast<unsigned char>(c)); });

    static constexpr std::array<std::pair<std::string_view, double>, 7> UNITS {
        {{"", 1.0}, {"m", 1.0}, {"mb", 1.0}, {"g", 1e3}, {"gb", 1e3}, {"t", 1e6}, {"tb", 1e6}}};
    const auto i(std::ranges::find(UNITS, std::string_view(unit), &std::pair<std::string_view, double>::first));
    if (i == UNITS.end())
        throw std::invalid_argument(std::format("Invalid unit of size `{}'", value));

    size = std::round(size * i->second);
    if (size > std::numeric_limits<int>::max()) // The database column is a (signed) INT
        throw std::out_of_range(std::format("Size `{}' too big", value));
    return static_cast<unsigned int>(size);
}

//-----------------------------------------------------------------------------
/// Gets the name of the film for the specified language. If such a name does
/// not exist, use the international one.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->type < b->type
//-----------------------------------------------------------------------------
const Glib::ustring& Film::getName(const std::string& lang) {
    contract_assert(name.contains(""));
    const auto i(name.find(lang));
    return ((i != name.end()) ? i->second : name[""]);
}

//-----------------------------------------------------------------------------
/// Sets the name of the film. If the name is empty, it is removed
/// If the default-name of the film is not set, use the passed value.
/// \param value New name of the film
//-----------------------------------------------------------------------------
void Film::setName(const Glib::ustring& value) { setName(value, name.contains("") ? currLang : ""); }

//-----------------------------------------------------------------------------
/// Sets the name in a certain language of the film. If the name is
/// empty, it is removed If the default-name of the film is not set,
/// use the passed value.
/// \param value New name of the film
/// \param lang Specification of language
//-----------------------------------------------------------------------------
void Film::setName(const Glib::ustring& value, const std::string& lang) {
    if (value.empty())
        name.erase(lang);
    else
        name[lang] = value;
}
