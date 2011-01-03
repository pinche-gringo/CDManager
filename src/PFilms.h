#ifndef PFILMS_H
#define PFILMS_H

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

#include "Film.h"
#include "Director.h"
#include "FilmList.h"

#include "NBPage.h"


// Forward declarations
class Genres;
class FilmList;
class LanguageImg;
class ImportFromIMDb;

namespace Gtk {
   class Dialog;
}


/**Class handling the films notebook-page
 */
class PFilms : public NBPage {
 public:
   PFilms (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres);
   virtual ~PFilms ();

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

   static HFilm findFilm (const std::vector<HDirector>& directors,
			    const YGP::Relation1_N<HDirector, HFilm>& relFilms,
			    unsigned int id);

   const FilmList& getFilmList () const { return films; }
   const std::vector<HDirector>& getDirectors () const { return directors; }
   const YGP::Relation1_N<HDirector, HFilm>& getRelFilms () const { return relFilms; }

 private:
   PFilms ();

   PFilms (const PFilms& other);
   const PFilms& operator= (const PFilms& other);

   void loadData (const std::string& lang);

   void selectLanguage ();
   void setLanguage (const std::string& lang);
   void changeLanguage (const std::string& lang);

   void directorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);
   void filmChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);

   void newDirector ();
   Gtk::TreeModel::iterator addDirector (HDirector& hDirector);
   void newFilm ();
   Gtk::TreeModel::iterator addFilm (HFilm& hFilm, Gtk::TreeIter pos);
   void deleteFilm (const Gtk::TreeIter& film);

   void undoFilm (const Undo& last);
   void undoDirector (const Undo& last);

   void importFromIMDb ();
   void importInfoFromIMDb ();

   void filmSelected ();
   HFilm findFilm (unsigned int id) const {
      return findFilm (directors, relFilms, id);
   }

   bool continousImportFilm (const Glib::ustring& director, const Glib::ustring& film,
			     const Glib::ustring& genre, const Glib::ustring& summary,
			     const std::string& image, ImportFromIMDb* dlg, std::vector<HFilm>* films);
   bool importFilm (const Glib::ustring& director, const Glib::ustring& film,
		    const Glib::ustring& genre, const Glib::ustring& summary, const std::string& image);
   bool importNextFilm (ImportFromIMDb* dlg, std::vector<HFilm>* films);

   bool onQueryTooltip (int x, int y, bool keyboard, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

   LanguageImg* imgLang;

   FilmList films;                              // GUI-element holding films

   // Model
   enum { DIRECTOR, FILM };
   std::vector<HDirector> directors;
   YGP::Relation1_N<HDirector, HFilm> relFilms;

   std::map<std::string, bool> loadedLangs;
};

#endif
