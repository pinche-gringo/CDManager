#ifndef MOVIE_H
#define MOVIE_H

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

#include <boost/shared_ptr.hpp>

#include <glibmm/ustring.h>

#include <YGP/AYear.h>
#include <YGP/Entity.h>
#include <XGP/XAttribute.h>

#include "CDType.h"
#include "Language.h"

class Movie;
typedef boost::shared_ptr<Movie> HMovie;

/**Class to hold a movie
 */
class Movie : public YGP::Entity {
 public:
   Movie ();
   Movie (const Movie& other);
   virtual ~Movie ();

   Movie& operator= (const Movie& other);

   unsigned long int    getId () const { return id; }
   const Glib::ustring& getName () { return getName (currLang); }
   const Glib::ustring& getName (const std::string& lang);
   const YGP::AYear&    getYear () const { return year; }
   unsigned int         getGenre () const { return genre; }
   int                  getType () const { return type; }
   const std::string&   getLanguage () const { return lang; }
   const std::string&   getTitles () const { return titles; }
   const Glib::ustring& getDescription () const { return summary; }
   const std::string&   getImage () const { return icon; }

   void setId       (unsigned long int value) { id = value; }
   void setName     (const Glib::ustring& value);
   void setName     (const Glib::ustring& value, const std::string& lang);
   const std::map<std::string, Glib::ustring>& getNames () { return name; }
   void setYear     (const YGP::AYear& value) { year = value; }
   void setYear     (const std::string& value) { year = value; }
   void setGenre    (unsigned int value) { genre = value; }
   void setType     (int value) throw (std::out_of_range) {
      CDType::getInstance ()[value];
      type = value; }
   void setType     (const std::string& value) throw (std::out_of_range) {
      type = CDType::getInstance ()[value]; }
   void setLanguage (const std::string& value) { lang = value; }
   void setTitles   (const std::string& value) { titles = value; }
   void setDescription (const Glib::ustring& value) { summary = value; }
   void setImage    (const std::string& value) { icon = value; }

   static Glib::ustring removeIgnored (const Glib::ustring& name);
   static bool compByName (const HMovie& a, const HMovie& b);
   static bool compByYear (const HMovie& a, const HMovie& b);
   static bool compByGenre (const HMovie& a, const HMovie& b);
   static bool compByMedia (const HMovie& a, const HMovie& b);

   static std::string currLang;

 private:
   unsigned long int id;       // %attrib%; ; 0
   std::map<std::string, Glib::ustring> name;     // %attrib%; Name
   YGP::AYear        year;     // %attrib%; Made
   unsigned int      genre;    // %attrib%; Genre; 0
   int               type;     // %attrib%; Media; 0
   std::string       lang;     // %attrib%; Lang
   std::string       titles;   // %attrib%; Subtitles
   Glib::ustring     summary;  // %attrib%; Description
   std::string       icon;     // %attrib%; Icon
};

#endif
