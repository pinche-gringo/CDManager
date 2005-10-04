#ifndef MOVIELIST_H
#define MOVIELIST_H

//$Id: MovieList.h,v 1.13 2005/10/04 16:23:12 markus Rel $

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
#include <gtkmm/liststore.h>

#include "Movie.h"
#include "Director.h"

#include "OOList.h"


class CellRendererList;


/**Class describing the columns in the CDType-model
 */
class TypeColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   TypeColumns () { add (type); }

   Gtk::TreeModelColumn<Glib::ustring> type;
};


/**Class for the columns in the movie list
 */
class MovieColumns : public OwnerObjectColumns {
 public:
   MovieColumns () : OwnerObjectColumns () {
      add (type); add (lang1); add (lang2); add (lang3); add (lang4);
      add (lang5); add (langs); add (sub1); add (sub2); add (sub3); add (sub4);
      add (sub5); add (sub6); add (titles); }

   Gtk::TreeModelColumn<Glib::ustring>              type;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > lang1;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > lang2;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > lang3;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > lang4;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > lang5;
   Gtk::TreeModelColumn<unsigned int>               langs;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub1;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub2;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub3;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub4;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub5;
   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > sub6;
   Gtk::TreeModelColumn<unsigned int>               titles;
};


/**Class to hold a list of movies
 */
class MovieList : public OwnerObjectList {
 public:
   MovieList (const std::map<unsigned int, Glib::ustring>& genres);
   virtual ~MovieList ();

   Gtk::TreeModel::Row append (const HDirector& director) {
      return OwnerObjectList::append (director); }
   Gtk::TreeModel::Row append (HMovie& movie, const Gtk::TreeModel::Row& director);

   HMovie getMovieAt (const Gtk::TreeIter iterator) const;
   HDirector getDirectorAt (const Gtk::TreeIter iterator) const {
      return getCelebrityAt (iterator); }

   void update (const std::string& lang);

   virtual void update (Gtk::TreeModel::Row& row);

   sigc::signal<void, const HMovie&> signalNameChanged;

 protected:
   virtual void setName (HEntity& object, const Glib::ustring& value);
   virtual void setYear (HEntity& object, const Glib::ustring& value);
   virtual void setGenre (HEntity& object, unsigned int value);

   virtual Glib::ustring getColumnName () const;
   virtual int sortEntity (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

   void valueChanged (const Glib::ustring& path, const Glib::ustring& value,
		      unsigned int column);

   virtual bool on_button_press_event (GdkEventButton* event);

   void setLanguage (Gtk::TreeModel::Row& row, const std::string& languages);
   void setTitles (Gtk::TreeModel::Row& row, const std::string& titles);

 private:
   MovieList (const MovieList& other);
   const MovieList& operator= (const MovieList& other);

   TypeColumns  colTypes;
   MovieColumns colMovies;

   Glib::RefPtr<Gtk::ListStore> mTypes;
};


#endif
