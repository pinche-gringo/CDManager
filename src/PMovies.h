#ifndef PMOVIES_H
#define PMOVIES_H

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
#include <vector>
#include <stdexcept>

#include <YGP/Relation.h>

#include "Movie.h"
#include "Director.h"
#include "MovieList.h"

#include "NBPage.h"


// Forward declarations
class Genres;
class MovieList;
class LanguageImg;

namespace Gtk {
   class Dialog;
}


/**Class handling the movies notebook-page
 */
class PMovies : public NBPage {
 public:
   PMovies (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres);
   virtual ~PMovies ();

   virtual void loadData ();
   virtual void saveData () throw (std::exception);
   virtual void getFocus ();
   virtual void addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction);
   virtual void removeMenu ();
   virtual void deleteSelection ();
   virtual void undo ();
   virtual void clear ();
   virtual void export2HTML (unsigned int fd, const std::string& lang);

   void addLanguageMenus (Glib::ustring& menu, Glib::RefPtr<Gtk::ActionGroup> grpAction);

   static HMovie findMovie (const std::vector<HDirector>& directors,
			    const YGP::Relation1_N<HDirector, HMovie>& relMovies,
			    unsigned int id);

   const MovieList& getMovieList () const { return movies; }
   const std::vector<HDirector>& getDirectors () const { return directors; }
   const YGP::Relation1_N<HDirector, HMovie>& getRelMovies () const { return relMovies; }

 private:
   PMovies ();

   PMovies (const PMovies& other);
   const PMovies& operator= (const PMovies& other);

   void loadData (const std::string& lang);

   void selectLanguage ();
   void setLanguage (const std::string& lang);
   void changeLanguage (const std::string& lang);

   void directorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);
   void movieChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);

   void newDirector ();
   Gtk::TreeModel::iterator addDirector (HDirector& hDirector);
   void newMovie ();
   Gtk::TreeModel::iterator addMovie (HMovie& hMovie, Gtk::TreeIter pos);
   void deleteMovie (const Gtk::TreeIter& movie);

   void undoMovie (const Undo& last);
   void undoDirector (const Undo& last);

   void importFromIMDb ();
   void importDescriptionFromIMDb ();
   static void closeDialog (int, const Gtk::Dialog* dlg);

   void movieSelected ();
   HMovie findMovie (unsigned int id) const {
      return findMovie (directors, relMovies, id);
   }

   bool importMovie (const Glib::ustring& director, const Glib::ustring& movie,
		     const Glib::ustring& genre);

   LanguageImg* imgLang;

   MovieList movies;                              // GUI-element holding movies

   // Model
   enum { DIRECTOR, MOVIE };
   std::vector<HDirector> directors;
   YGP::Relation1_N<HDirector, HMovie> relMovies;

   std::map<std::string, bool> loadedLangs;
};

#endif
