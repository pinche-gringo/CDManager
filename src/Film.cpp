//PROJECT     : CDManager
//SUBSYSTEM   : Film
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011

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


#include <glibmm/ustring.h>

#include <YGP/Check.h>

// #include <XGP/XAttribute.h>  // Needed for specialization of YGP::Attribute for Glib::ustring

#include "Words.h"
#include "Film.h"


// Specialization of YGP::Attribute for the name-map
namespace YGP {

template <> inline
bool Attribute<std::map<std::string, Glib::ustring> >::assignFromString (const char* value) const {
   Check3 (value);
   attr_[Film::currLang] = value;
   return true;
}
template <> inline
std::string Attribute<std::map<std::string, Glib::ustring> >::getValue () const {
   return (attr_.find (Film::currLang) == attr_.end ()) ? attr_[""] : attr_[Film::currLang]; }
template <> inline
std::string Attribute<std::map<std::string, Glib::ustring> >::getFormattedValue () const {
   return getValue (); }

}


#include "Film.meta"


std::string Film::currLang;


//-----------------------------------------------------------------------------
/// Copyonstructor
//-----------------------------------------------------------------------------
Film::Film (const Film& other)
   : id (other.id), name (other.name), year (other.year),
     genre (other.genre), type (other.type), lang (other.lang),
     titles (other.titles) {
 }


//-----------------------------------------------------------------------------
/// Assignment operator
/// \param other Object to assign
/// \returns Film& Reference to self
//-----------------------------------------------------------------------------
Film& Film::operator= (const Film& other) {
   if (this != &other) {
      if (!id)
	 id = other.id;

      name = other.name;
      year = other.year;
      genre = other.genre;
      type = other.type;
      lang = other.lang;
      titles = other.titles;
   }
   return *this;
}

//-----------------------------------------------------------------------------
/// Removes the leading articles of the names.
/// \param name Name to manipulate
/// \returns Glib::ustring Changed name
//-----------------------------------------------------------------------------
Glib::ustring Film::removeIgnored (const Glib::ustring& name) {
   return Words::removeArticle (name);
}

//-----------------------------------------------------------------------------
/// Sorts films by name with respect to the "logical" sense (e.g. ignore
/// articles)
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->name < b->name
//-----------------------------------------------------------------------------
bool Film::compByName (const HFilm& a, const HFilm& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   Glib::ustring aname (removeIgnored (a->getName ()));
   Glib::ustring bname (removeIgnored (b->getName ()));
   int rc (aname.compare (bname));
   if (!rc)
      rc = a->year < b->year;
   return rc < 0;
}

//-----------------------------------------------------------------------------
/// Sorts films by year.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->year < b->year
//-----------------------------------------------------------------------------
bool Film::compByYear (const HFilm& a, const HFilm& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   int rc (a->year - b->year);
   if (!rc) {
      Glib::ustring aname (removeIgnored (a->getName ()));
      Glib::ustring bname (removeIgnored (b->getName ()));
      rc = aname.compare (bname);
   }
   return rc < 0;
}

//-----------------------------------------------------------------------------
/// Sorts films by genre.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->genre < b->genre
//-----------------------------------------------------------------------------
bool Film::compByGenre (const HFilm& a, const HFilm& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   int rc (a->genre - b->genre);
   return rc ? (rc < 0) : compByName (a, b);
}

//-----------------------------------------------------------------------------
/// Sorts films by media.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->type < b->type
//-----------------------------------------------------------------------------
bool Film::compByMedia (const HFilm& a, const HFilm& b) {
   Check1 (a.isDefined ()); Check1 (b.isDefined ());
   int rc (a->type - b->type);
   return rc ? (rc < 0) : compByName (a, b);
}

//-----------------------------------------------------------------------------
/// Gets the name of the film for the specified language. If such a name does
/// not exist, use the international one.
/// \param a First film
/// \param b Second film
/// \returns bool True, if a->type < b->type
//-----------------------------------------------------------------------------
const Glib::ustring& Film::getName (const std::string& lang) {
   Check2 (name.find ("") != name.end ());
   std::map<std::string, Glib::ustring>::const_iterator i (name.find (lang));
   return ((i != name.end ()) ? i->second : name[""]);
}

//-----------------------------------------------------------------------------
/// Sets the name of the film. If the name is empty, it is removed
/// If the default-name of the film is not set, use the passed value.
/// \param value New name of the film
//-----------------------------------------------------------------------------
void Film::setName (const Glib::ustring& value) {
   setName (value, (name.find ("") != name.end ()) ? currLang : "");
}

//-----------------------------------------------------------------------------
/// Sets the name in a certain language of the film. If the name is
/// empty, it is removed If the default-name of the film is not set,
/// use the passed value.
/// \param value New name of the film
/// \param lang Specification of language
//-----------------------------------------------------------------------------
void Film::setName (const Glib::ustring& value, const std::string& lang) {
   if (value.empty ()) {
      std::map<std::string, Glib::ustring>::iterator i (name.find (currLang));
      if (i != name.end ()) {
	 name.erase (i);
	 return;
      }
   }
   name[lang] = value;
}
