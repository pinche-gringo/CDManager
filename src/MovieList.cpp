//$Id: MovieList.cpp,v 1.1 2004/11/17 20:37:53 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : src
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 31.10.2004
//COPYRIGHT   : Anticopyright (A) 2004

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


#include <cdmgr-cfg.h>

#include <cerrno>
#include <cstdlib>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <XGP/XValue.h>

#include "RendererList.h"

#include "MovieList.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
MovieList::MovieList (const std::map<unsigned int, Glib::ustring>& genres)
   : genres (genres), mMovies (Gtk::TreeStore::create (colMovies)) {
   TRACE9 ("MovieList::MovieList (const std::map<unsigned int, Glib::ustring>&)");

   set_model (mMovies);

   append_column (_("Director/Movie"), colMovies.name);
   append_column (_("Year"), colMovies.year);

   set_headers_clickable ();

   for (unsigned int i (0); i < 2; ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
      column->set_sort_column (i + 1);
      column->set_resizable ();

      Check3 (get_column_cell_renderer (i));
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
      Check3 (typeid (*r) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (r));
      rText->property_editable () = true;
      rText->signal_edited ().connect
	 (bind (mem_fun (*this, &MovieList::valueChanged), i));
   }

   CellRendererList* const renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn
				      (_("Genre"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text(), colMovies.genre);

   column->set_sort_column (3);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &MovieList::valueChanged), 2));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MovieList::~MovieList () {
   TRACE9 ("MovieList::~MovieList ()");
}


//-----------------------------------------------------------------------------
/// Appends a movie to the list
/// \param movie: Movie to add
/// \param director: Director of the movie
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row MovieList::append (HMovie& movie,
					const Gtk::TreeModel::Row& director) {
   TRACE3 ("MovieList::append (HMovie&, Gtk::TreeModel::Row) - "
	   << (movie.isDefined () ? movie->name.c_str () : "None"));
   Check1 (movie.isDefined ());

   Gtk::TreeModel::Row newMovie (*mMovies->append (director.children ()));
   newMovie[colMovies.entry] = YGP::Handle<YGP::Entity>::cast (movie);
   newMovie[colMovies.name] = movie->name;
   if (movie->year)
      newMovie[colMovies.year] = movie->year;

   std::map<unsigned int, Glib::ustring>::const_iterator g
      (genres.find (movie->genre));
   if (g == genres.end ())
      g = genres.begin ();
   newMovie[colMovies.genre] = g->second;

   return newMovie;
}

//-----------------------------------------------------------------------------
/// Appends an director to the list
/// \param movie: Movie to add
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row MovieList::append (const HDirector& director) {
   TRACE3 ("MovieList::append (HDirector&) - " << (director.isDefined () ? director->name.c_str () : "None"));
   Check1 (director.isDefined ());

   Gtk::TreeModel::Row newDirector (*mMovies->append ());
   newDirector[colMovies.entry] = YGP::Handle<YGP::Entity>::cast (director);
   newDirector[colMovies.name] = director->name;

   return newDirector;
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void MovieList::valueChanged (const Glib::ustring& path,
			       const Glib::ustring& value, unsigned int column) {
   TRACE9 ("MovieList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);

   Gtk::TreeModel::Row row (*mMovies->get_iter (Gtk::TreeModel::Path (path)));

   try {
      if (row.parent ()) {
	 HMovie movie (getMovieAt (row));
	 signalMovieChanged.emit (movie);
	 switch (column) {
	 case 0:
	    row[colMovies.name] = movie->name= value;
	    break;
	 case 1:
	    row[colMovies.year] = movie->year = value;
	    break;
	 case 2: {
	    for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
		 g != genres.end (); ++g)
	       if (g->second == value) {
		  movie->genre = g->first;
		  row[colMovies.genre] = value;
		  signalMovieGenreChanged.emit (movie);
		  return;
	       }
	    throw (std::invalid_argument (_("Unknown genre!")));
	    break; }
	 default:
	    Check3 (0);
	 } // endswitch
      } // endif movie edited
      else {
	 if (!column) {
	    HDirector director (getDirectorAt (row));
	    Check3 (director.isDefined ());

	    row[colMovies.name] = director->name= value;
	    signalDirectorChanged.emit (director);
	 }
      } // end-else director edited
   } // end-try
   catch (std::exception& e) {
      YGP::StatusObject obj (YGP::StatusObject::ERROR, e.what ());
      obj.generalize (_("Invalid value!"));

      XGP::MessageDlg* dlg (XGP::MessageDlg::create (obj));
      dlg->set_title (PACKAGE);
      dlg->get_window ()->set_transient_for (this->get_window ());
   }
}

//-----------------------------------------------------------------------------
/// Sets the genres list
//-----------------------------------------------------------------------------
void MovieList::updateGenres () {
   TRACE9 ("MovieList::updateGenres () - Genres: " << genres.size ());

   Check3 (get_column_cell_renderer (2));
   Gtk::CellRenderer* r (get_column_cell_renderer (2)); Check3 (r);
   Check3 (typeid (*r) == typeid (CellRendererList));
   CellRendererList* renderer (dynamic_cast<CellRendererList*> (r));

   for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	g != genres.end (); ++g)
      renderer->append_list_item (g->second);
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HMovie) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HMovie: Handle of the selected line
//-----------------------------------------------------------------------------
HMovie MovieList::getMovieAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   YGP::Handle<YGP::Entity> hRec ((*iter)[colMovies.entry]);
   Check3 (hRec.isDefined ());
   HMovie movie (HMovie::cast (hRec));
   TRACE7 ("CDManager::getMovieAt (const Gtk::TreeIter&) - Selected movie: " <<
	   movie->id << '/' << movie->name);
   return movie;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HMovie) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HMovie: Handle of the selected line
//-----------------------------------------------------------------------------
HDirector MovieList::getDirectorAt (const Gtk::TreeIter iter) const {
   Check2 (!(*iter)->parent ());
   YGP::Handle<YGP::Entity> hDirector ((*iter)[colMovies.entry]);
   Check3 (hDirector.isDefined ());
   HDirector director (HDirector::cast (hDirector));
   TRACE7 ("CDManager::getDirectorAt (const Gtk::TreeIter&) - Selected director: " <<
	   director->id << '/' << director->name);
   return director;
}
