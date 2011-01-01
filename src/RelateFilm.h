#ifndef RELATEFILM_H
#define RELATEFILM_H

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


#include <vector>

#include <boost/shared_ptr.hpp>

#include <glibmm/refptr.h>

#include "Actor.h"
#include "Film.h"
#include "Director.h"

#include <XGP/XDialog.h>

namespace Gtk {
   class Label;
   class Table;
   class Button;
   class TreeView;
   class TreeStore;
   class TreeViewColumn;
}


typedef boost::shared_ptr<YGP::Entity> HEntity;


/**Dialog to permit connecting films to an actor
 */
class RelateFilm : public XGP::XDialog {
 private:
   /**Class describing the columns in the film-view
    */
   struct FilmColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      FilmColumns () { add (hFilm); add (film); }

      Gtk::TreeModelColumn<HFilm>  hFilm;
      Gtk::TreeModelColumn<Glib::ustring> film;
   };


   /**Class describing the columns in the all films-view
    */
   class AllFilmColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      AllFilmColumns () { add (entry); add (name); }

      Gtk::TreeModelColumn<HEntity> entry;
      Gtk::TreeModelColumn<Glib::ustring> name;
   };

 public:
   virtual ~RelateFilm ();

   static RelateFilm* create (const HActor& actor, const std::vector<HFilm>& films,
			       const Glib::RefPtr<Gtk::TreeStore> allFilms) {
      RelateFilm* dlg (new RelateFilm (actor, films, allFilms));
      dlg->signal_response ().connect (mem_fun (*dlg, &RelateFilm::free));
      return dlg;
   }
   static RelateFilm* create (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allFilms) {
      RelateFilm* dlg (new RelateFilm (actor, allFilms));
      dlg->signal_response ().connect (mem_fun (*dlg, &RelateFilm::free));
      return dlg;
   }

   sigc::signal<void, const HActor&, const std::vector<HFilm>&> signalRelateFilms;

 private:
   RelateFilm (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allFilms);
   RelateFilm (const HActor& actor, const std::vector<HFilm>& films,
		const Glib::RefPtr<Gtk::TreeStore> allFilms);

   void init ();

   //Prohibited manager functions
   RelateFilm ();
   RelateFilm (const RelateFilm& other);
   const RelateFilm& operator= (const RelateFilm& other);

   virtual void okEvent ();

   void insertFilm (const HFilm& film);
   void addFilm (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*);
   void removeFilm (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*);

   void addSelected ();
   void removeSelected ();

   void filmsSelected ();
   void allFilmsSelected ();

   FilmColumns    colFilms;
   AllFilmColumns colAllFilms;

   Glib::RefPtr<Gtk::ListStore> mFilms;
   Glib::RefPtr<Gtk::TreeStore> availFilms;

   Gtk::Button& addFilms;
   Gtk::Button& removeFilms;

   Gtk::TreeView& lstFilms;
   Gtk::TreeView& lstAllFilms;

   HActor actor;
};

#endif
