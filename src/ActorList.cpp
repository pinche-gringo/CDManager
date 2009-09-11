//$Id: ActorList.cpp,v 1.10 2007/02/09 12:15:00 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Actor
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.10 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.09.2005
//COPYRIGHT   : Copyright (C) 2005 - 2007, 2009

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

#include <gtkmm/cellrenderercombo.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/XValue.h>
#include <XGP/MessageDlg.h>

#include "Movie.h"
#include "Actor.h"

#include "ActorList.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
ActorList::ActorList (const Genres& genres) : genres (genres) {
   TRACE9 ("ActorList::ActorList (const Genres&)");
   mOwnerObjects = Gtk::TreeStore::create (colActors);

   set_model (mOwnerObjects);

   append_column (_("Actors/Movies"), colActors.name);
   append_column (_("Year"), colActors.year);
   append_column (_("Genre"), colActors.genre);

   Check3 (get_columns ().size () == 3);
   unsigned int index[] = { colActors.name.index (), colActors.year.index () };
   for (unsigned int i (0); i < (sizeof (index) / sizeof (*index)); ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
      column->set_sort_column (index[i]);
      column->set_resizable ();

      Check3 (get_column_cell_renderer (i));
      Check3 (typeid (*get_column_cell_renderer (i)) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (get_column_cell_renderer (i)));
      column->add_attribute (rText->property_editable(), colActors.editable);
      rText->signal_edited ().connect (bind (mem_fun (*this, &ActorList::valueChanged), i));
   }

   mOwnerObjects->set_sort_func (colActors.name, mem_fun (*this, &ActorList::sortByName));
   mOwnerObjects->set_sort_func (colActors.year, mem_fun (*this, &ActorList::sortByYear));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ActorList::~ActorList () {
   TRACE9 ("ActorList::~ActorList ()");
}


//-----------------------------------------------------------------------------
/// Appends a line to the list
/// \param entity: Entity to add
/// \param pos: Position in model for insert
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeRow ActorList::insert (const HEntity& entity, const Gtk::TreeIter& pos) {
   TRACE7 ("ActorList::insert (const HEntity&, const Gtk::TreeIter&)");
   Check1 (entity);

   Gtk::TreeRow newRow (*mOwnerObjects->insert (pos));
   newRow[colActors.entry] = entity;
   update (newRow);
   return newRow;
}

//-----------------------------------------------------------------------------
/// Appends a actor to the list
/// \param entity: Entity to add
/// \param artist: Actor starring in the movie
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeRow ActorList::append (const HEntity& entity, const Gtk::TreeIter& pos) {
   TRACE7 ("ActorList::append (const HEntity&, const Gtk::TreeRow&)");
   Check1 (entity);

   Gtk::TreeRow newLine (*mOwnerObjects->append (pos->children ()));
   newLine[colActors.entry] = entity;
   update (newLine);
   return newLine;
}

//-----------------------------------------------------------------------------
/// Sets the values of the line to the values of the stored entity
/// \param row: Row to update
//-----------------------------------------------------------------------------
void ActorList::update (Gtk::TreeRow& row) {
   HEntity obj (row[colActors.entry]);
   HMovie movie (boost::dynamic_pointer_cast<Movie> (obj));
   if (movie) {
      row[colActors.name] = movie->getName ();
      row[colActors.year] = movie->getYear ().toString ();

      unsigned int g (movie->getGenre ());
      if (g >= genres.size ())
	 g = 0;
      row[colActors.genre] = genres[g];
      row[colActors.editable] = false;
   }
   else {
      HActor actor (boost::dynamic_pointer_cast<Actor> (obj)); Check3 (actor);
      row[colActors.name] = actor->getName ();
      row[colActors.year] = actor->getLifespan ();
      row[colActors.editable] = true;
   }
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void ActorList::valueChanged (const Glib::ustring& path,
			      const Glib::ustring& value, unsigned int column) {
   TRACE9 ("ActorList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);
   Check2 (column < 3);

   Gtk::TreeModel::Row row (*mOwnerObjects->get_iter (Gtk::TreeModel::Path (path)));
   Glib::ustring oldValue;

   try {
      HEntity hEntity (row[colActors.entry]);
      HActor actor (boost::dynamic_pointer_cast<Actor> (hEntity)); Check3 (actor);

      switch (column) {
      case 0:
	 if (value.size ()) {
	    Gtk::TreeModel::const_iterator i (findName (value));
	    if ((i != row) && (i != mOwnerObjects->children ().end ())) {
	       Glib::ustring e (_("Entry `%1' already exists!"));
	       e.replace (e.find ("%1"), 2, value);
	       throw (YGP::InvalidValue (e));
	    }
	 }
	 actor->setName (value);
	 oldValue = row[colActors.name];
	 row[colActors.name] = value;
	 break;

      case 1:
	 actor->setLifespan (value);
	 oldValue = row[colActors.year];
	 row[colActors.year] = value;
	 break;
      } // end-switch

      if (value != oldValue)
	 signalActorChanged.emit (row, column, oldValue);
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
/// Selects the passed row (as only one) and centers it
/// \param i: Iterator to row to select
//-----------------------------------------------------------------------------
void ActorList::selectRow (const Gtk::TreeModel::const_iterator& i) {
   Glib::RefPtr<Gtk::TreeSelection> sel (get_selection ());
   Gtk::TreePath path (mOwnerObjects->get_path (i));
   scroll_to_row (path, 0.5);
   set_cursor (path);
   sel->select (path);
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the line having the name stored. Only editable lines
/// are considered
/// \param name: Name of object to find
/// \param level: Level of recursion (0: None)
/// \param begin: Start object
/// \param end: End object
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or mOwnerObjects->children ().end ().
//-----------------------------------------------------------------------------
Gtk::TreeIter ActorList::findName (const Glib::ustring& name, unsigned int level,
				   Gtk::TreeIter begin, Gtk::TreeIter end) const {
   while (begin != end) {
      Gtk::TreeModel::Row actRow (*begin);
      if ((actRow[colActors.editable] == true) && (name == actRow[colActors.name]))
	 return begin;

      if (level && begin->children ().size ()) {
	 Gtk::TreeIter res (findName (name, level - 1, begin->children ().begin (),
				      begin->children ().end ()));
	 if (res != mOwnerObjects->children ().end ())
	    return res;
      }
      ++begin;
   } // end-while
   return mOwnerObjects->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the line having entry stored
/// \param entry: Entry to find
/// \param level: Level of recursion (0: None)
/// \param begin: Start object
/// \param end: End object
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or mOwnerObjects->children ().end ().
//-----------------------------------------------------------------------------
Gtk::TreeIter ActorList::findEntity (const HEntity& entry, unsigned int level,
				     Gtk::TreeIter begin, Gtk::TreeIter end) const {
   while (begin != end) {
      Gtk::TreeModel::Row actRow (*begin);
      if (entry == (HEntity)actRow[colActors.entry])
	 return begin;

      if (level && begin->children ().size ()) {
	 Gtk::TreeIter res (findEntity (entry, level - 1, begin->children ().begin (),
					begin->children ().end ()));
	 if (res != mOwnerObjects->children ().end ())
	    return res;
      }
      ++begin;
   } // end-while
   return mOwnerObjects->children ().end ();
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring first names,
/// articles, ...)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortByName (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const {
   Check2 (a->parent () == b->parent ());
   Gtk::TreeRow ra (*a);
   Gtk::TreeRow rb (*b);

   HEntity entity (ra[colActors.entry]);
   int rc ((typeid (*entity.get ()) == typeid (Actor))
	   ? Actor::removeIgnored (ra[colActors.name]).compare (Actor::removeIgnored (rb[colActors.name]))
	   : Movie::removeIgnored (ra[colActors.name]).compare (Movie::removeIgnored (rb[colActors.name])));
   if (!rc)
      rc = ((Glib::ustring)ra[colActors.name]).compare ((Glib::ustring)rb[colActors.name]);
   return rc;
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the year
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortByYear (const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) const {
   Check2 (a->parent () == b->parent ());
   Gtk::TreeRow ra (*a);
   Gtk::TreeRow rb (*b);
   YGP::AYear ya;
   YGP::AYear yb;

   HEntity entity (ra[colActors.entry]);
   if (typeid (*entity) == typeid (Actor)) {
      ya = boost::dynamic_pointer_cast<Actor> (entity)->getBorn ();
      entity = rb[colActors.entry];
      yb = boost::dynamic_pointer_cast<Actor> (entity)->getBorn ();
   }
   else {
      ya = boost::dynamic_pointer_cast<Movie> (entity)->getYear ();
      entity = rb[colActors.entry];
      yb = boost::dynamic_pointer_cast<Movie> (entity)->getYear ();
   }
   return ya.compare (yb);
}
