#ifndef RELATEMOVIE_H
#define RELATEMOVIE_H

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
#include "Movie.h"
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


/**Dialog to permit connecting movies to an actor
 */
class RelateMovie : public XGP::XDialog {
 private:
   /**Class describing the columns in the movie-view
    */
   struct MovieColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      MovieColumns () { add (hMovie); add (movie); }

      Gtk::TreeModelColumn<HMovie>  hMovie;
      Gtk::TreeModelColumn<Glib::ustring> movie;
   };


   /**Class describing the columns in the all movies-view
    */
   class AllMovieColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      AllMovieColumns () { add (entry); add (name); }

      Gtk::TreeModelColumn<HEntity> entry;
      Gtk::TreeModelColumn<Glib::ustring> name;
   };

 public:
   virtual ~RelateMovie ();

   static RelateMovie* create (const HActor& actor, const std::vector<HMovie>& movies,
			       const Glib::RefPtr<Gtk::TreeStore> allMovies) {
      RelateMovie* dlg (new RelateMovie (actor, movies, allMovies));
      dlg->signal_response ().connect (mem_fun (*dlg, &RelateMovie::free));
      return dlg;
   }
   static RelateMovie* create (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allMovies) {
      RelateMovie* dlg (new RelateMovie (actor, allMovies));
      dlg->signal_response ().connect (mem_fun (*dlg, &RelateMovie::free));
      return dlg;
   }

   sigc::signal<void, const HActor&, const std::vector<HMovie>&> signalRelateMovies;

 private:
   RelateMovie (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allMovies);
   RelateMovie (const HActor& actor, const std::vector<HMovie>& movies,
		const Glib::RefPtr<Gtk::TreeStore> allMovies);

   void init ();

   //Prohibited manager functions
   RelateMovie ();
   RelateMovie (const RelateMovie& other);
   const RelateMovie& operator= (const RelateMovie& other);

   virtual void okEvent ();

   void insertMovie (const HMovie& movie);
   void addMovie (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*);
   void removeMovie (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*);

   void addSelected ();
   void removeSelected ();

   void moviesSelected ();
   void allMoviesSelected ();

   MovieColumns    colMovies;
   AllMovieColumns colAllMovies;

   Glib::RefPtr<Gtk::ListStore> mMovies;
   Glib::RefPtr<Gtk::TreeStore> availMovies;

   Gtk::Button& addMovies;
   Gtk::Button& removeMovies;

   Gtk::TreeView& lstMovies;
   Gtk::TreeView& lstAllMovies;

   HActor actor;
};

#endif
