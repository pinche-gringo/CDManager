//PROJECT     : CDManager
//SUBSYSTEM   : Actor
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 2005-10-18
//COPYRIGHT   : Copyright (C) 2005, 2009, 2010

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


#include <cdmgr-cfg.h>

#include <gtkmm/stock.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "RelateMovie.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose movies should be displayed/changed
/// \param movies: Movies already connected with the actor
/// \param allMovies: All available movies
//-----------------------------------------------------------------------------
RelateMovie::RelateMovie (const HActor& actor, const std::vector<HMovie>& movies,
			  const Glib::RefPtr<Gtk::TreeStore> allMovies)
   : XGP::XDialog (OKCANCEL),
     mMovies (Gtk::ListStore::create (colMovies)),
     availMovies (allMovies),
     addMovies (*manage (new Gtk::Button)),
     removeMovies (*manage (new Gtk::Button)),
     lstMovies (*manage (new Gtk::TreeView)),
     lstAllMovies (*manage (new Gtk::TreeView)),
     actor (actor) {
   TRACE9 ("RelateMovie::RelateMovie (const HActor&, const std::vector<HMovie>&, const Glib::RefPtr<Gtk::TreeStore>)");
   Check3 (actor);

   for (std::vector<HMovie>::const_iterator i (movies.begin ()); i != movies.end (); ++i)
      insertMovie (*i);

   init ();
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose movies should be displayed/changed
/// \param movies: Movies already connected with the actor
/// \param allMovies: All available movies
//-----------------------------------------------------------------------------
RelateMovie::RelateMovie (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allMovies)
   : XGP::XDialog (OKCANCEL),
     mMovies (Gtk::ListStore::create (colMovies)),
     availMovies (allMovies),
     addMovies (*manage (new Gtk::Button)),
     removeMovies (*manage (new Gtk::Button)),
     lstMovies (*manage (new Gtk::TreeView)),
     lstAllMovies (*manage (new Gtk::TreeView)),
     actor (actor) {
   TRACE9 ("RelateMovie::RelateMovie (const HActor&, const Glib::RefPtr<Gtk::TreeStore>)");
   init ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RelateMovie::~RelateMovie () {
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void RelateMovie::okEvent () {
   Check3 (actor);

   std::vector<HMovie> movies;
   for (Gtk::TreeModel::const_iterator i (mMovies->children ().begin ());
	i != mMovies->children ().end (); ++i)
      movies.push_back ((*i)[colMovies.hMovie]);
   signalRelateMovies.emit (actor, movies);
}

//-----------------------------------------------------------------------------
/// Adds a movie to the ones starring the actor
/// \param path: Path to movie to add
//-----------------------------------------------------------------------------
void RelateMovie::addMovie (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
   TRACE7 ("RelateMovie::addMovie (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");
   Check3 (lstAllMovies.get_model ());

   Gtk::TreeIter sel (availMovies->get_iter (path)); Check3 (sel);
   if (sel->parent ()) {
      HEntity entry ((*sel)[colAllMovies.entry]); Check3 (entry);
      HMovie movie (boost::dynamic_pointer_cast<Movie> (entry)); Check3 (movie);
      TRACE9 ("RelateMovie::addMovie (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << movie->getName ());
      insertMovie (movie);
   }
   else {
      for (Gtk::TreeIter i (sel->children ().begin ()); i != sel->children ().end (); ++i) {
	 HEntity entry ((*i)[colAllMovies.entry]); Check3 (entry);
	 HMovie movie (boost::dynamic_pointer_cast<Movie> (entry)); Check3 (movie);
	 TRACE9 ("RelateMovie::addMovie (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << movie->getName ())
	 insertMovie (movie);
      }
   }
}

//-----------------------------------------------------------------------------
/// Adds a movie to the ones starring the actor
/// \param path: Path to movie to add
//-----------------------------------------------------------------------------
void RelateMovie::removeMovie (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
   TRACE9 ("RelateMovie::removeMovie (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");

   mMovies->erase (mMovies->get_iter (path));
}

//-----------------------------------------------------------------------------
/// Adds the passed movie/director to the movie-list for the actor
/// \param movie: Movie to add
/// \param director: Director of the movie
//-----------------------------------------------------------------------------
void RelateMovie::insertMovie (const HMovie& movie) {
   TRACE9 ("RelateMovie::insertMovie (const HMovie&) - " << (movie ? movie->getName ().c_str () : ""));
   Check1 (movie);

   // Check that movie does not exist
   for (Gtk::TreeModel::const_iterator i (mMovies->children ().begin ());
	i != mMovies->children ().end (); ++i)
      if (movie == (HMovie)(*i)[colMovies.hMovie])
	 return;

   Gtk::TreeModel::Row newMovie (*mMovies->append ());
   newMovie[colMovies.hMovie] = movie;
   newMovie[colMovies.movie] = movie->getName ();
}

//-----------------------------------------------------------------------------
/// Adds all selected movies
//-----------------------------------------------------------------------------
void RelateMovie::addSelected () {
   TRACE9 ("RelateMovie::addSelected ()");

   Glib::RefPtr<Gtk::TreeSelection> movieSel (lstAllMovies.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (movieSel->get_selected_rows ());
   for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
	i != list.end (); ++i)
      addMovie (*i, NULL);
}

//-----------------------------------------------------------------------------
/// Remove all selected movies
//-----------------------------------------------------------------------------
void RelateMovie::removeSelected () {
   TRACE9 ("RelateMovie::removeSelected ()");

   Glib::RefPtr<Gtk::TreeSelection> movieSel (lstMovies.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (movieSel->get_selected_rows ());
   for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
	i != list.end (); ++i)
      removeMovie (*i, NULL);
}

//-----------------------------------------------------------------------------
/// Callback after changing the movies-selection. Used to enable/disable the
/// remove-button
//-----------------------------------------------------------------------------
void RelateMovie::moviesSelected () {
   removeMovies.set_sensitive (lstMovies.get_selection ()->get_selected_rows ().size ());
}

//-----------------------------------------------------------------------------
/// Callback after changing the allMovies-selection. Used to enable/disable the
/// add-button
//-----------------------------------------------------------------------------
void RelateMovie::allMoviesSelected () {
   addMovies.set_sensitive (lstAllMovies.get_selection ()->get_selected_rows ().size ());
}

//-----------------------------------------------------------------------------
/// Inititalizes the class
//-----------------------------------------------------------------------------
void RelateMovie::init () {
   Check2 (actor);
   Glib::ustring title (_("Movies starring %1"));
   title.replace (title.find ("%1"), 2, actor->getName ());
   set_title (title);
   set_default_size (580, 450);

   Check1 (mMovies);
   lstMovies.set_model (mMovies);
   lstAllMovies.set_model (availMovies);

   Gtk::ScrolledWindow& scrlMovies (*manage (new Gtk::ScrolledWindow));
   Gtk::ScrolledWindow& scrlAllMovies (*manage (new Gtk::ScrolledWindow));
   scrlMovies.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlAllMovies.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlMovies.add (lstMovies);
   scrlAllMovies.add (lstAllMovies);
   scrlMovies.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlAllMovies.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   lstMovies.append_column (_("Movie"), colMovies.movie);
   lstAllMovies.append_column (_("Available directors/movies"), colAllMovies.name);

   Glib::RefPtr<Gtk::TreeSelection> sel (lstMovies.get_selection ());
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &RelateMovie::moviesSelected));
   moviesSelected ();

   sel = lstAllMovies.get_selection ();
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &RelateMovie::allMoviesSelected));
   allMoviesSelected ();

   lstMovies.signal_row_activated ().connect (mem_fun (*this, &RelateMovie::removeMovie));
   lstAllMovies.signal_row_activated ().connect (mem_fun (*this, &RelateMovie::addMovie));
   lstAllMovies.expand_all ();

   Gtk::Box& bbox (*manage (new Gtk::VBox));
   addMovies.add (*manage (new Gtk::Image (Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_BUTTON)));
   removeMovies.add (*manage (new Gtk::Image (Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON)));

   bbox.pack_start (addMovies, Gtk::PACK_SHRINK, 5);
   bbox.pack_start (removeMovies, Gtk::PACK_SHRINK, 5);
   addMovies.signal_clicked ().connect (mem_fun (*this, &RelateMovie::addSelected));
   removeMovies.signal_clicked ().connect (mem_fun (*this, &RelateMovie::removeSelected));

   Gtk::HBox& box (*manage (new Gtk::HBox));
   box.pack_start (scrlMovies, Gtk::PACK_EXPAND_WIDGET, 5);
   box.pack_start (bbox, Gtk::PACK_SHRINK, 5);
   box.pack_start (scrlAllMovies, Gtk::PACK_EXPAND_WIDGET, 5);
   get_vbox ()->pack_start (box, Gtk::PACK_EXPAND_WIDGET);

   show_all_children ();
   show ();
}
