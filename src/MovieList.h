#ifndef MOVIELIST_H
#define MOVIELIST_H

//$Id: MovieList.h,v 1.2 2004/11/25 23:20:21 markus Exp $

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

#include "Movie.h"
#include "Director.h"


class CellRendererList;


/**Class describing the columns in the movie-model
 */
class MovieColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   MovieColumns () {
      add (entry); add (name); add (year); add (genre); }

   Gtk::TreeModelColumn<YGP::Handle<YGP::Entity> > entry;
   Gtk::TreeModelColumn<Glib::ustring>             name;
   Gtk::TreeModelColumn<Glib::ustring>             year;
   Gtk::TreeModelColumn<Glib::ustring>             genre;
};


/**Class to hold a list of movies
 */
class MovieList : public Gtk::TreeView {
 public:
   MovieList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~MovieList ();

   Gtk::TreeModel::Row append (const HDirector& director);
   Gtk::TreeModel::Row append (HMovie& movie, const Gtk::TreeModel::Row& director);
   void clear () { mMovies->clear (); }

   void updateGenres ();

   sigc::signal<void, const HDirector&> signalDirectorChanged;
   sigc::signal<void, const HMovie&> signalMovieChanged;
   sigc::signal<void, const HMovie&> signalMovieGenreChanged;

   Glib::RefPtr<Gtk::TreeStore> getModel () const { return mMovies; }
   HMovie getMovieAt (const Gtk::TreeIter iterator) const;
   HDirector getDirectorAt (const Gtk::TreeIter iterator) const;

 protected:
   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

 private:
   MovieList (const MovieList& other);
   const MovieList& operator= (const MovieList& other);

   Glib::ustring getLiveSpan (const HDirector& director);

   const std::map<unsigned int, Glib::ustring>& genres;

   MovieColumns colMovies;
   Glib::RefPtr<Gtk::TreeStore> mMovies;
};


#endif
