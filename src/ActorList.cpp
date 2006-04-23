//$Id: ActorList.cpp,v 1.7 2006/04/23 23:00:00 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Actor
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.7 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.09.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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
Gtk::TreeRow ActorList::insert (const YGP::HEntity& entity, const Gtk::TreeIter& pos) {
   TRACE7 ("ActorList::insert (const YGP::HEntity&, const Gtk::TreeIter&)");
   Check1 (entity.isDefined ());

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
Gtk::TreeRow ActorList::append (const YGP::HEntity& entity, const Gtk::TreeIter& pos) {
   TRACE7 ("ActorList::append (const YGP::HEntity&, const Gtk::TreeRow&)");
   Check1 (entity.isDefined ());

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
   YGP::HEntity entity (row[colActors.entry]);
   HMovie movie (HMovie::castDynamic (entity));
   if (movie.isDefined ()) {
      row[colActors.name] = movie->getName ();
      row[colActors.year] = movie->getYear ().toString ();

      Genres::const_iterator g (genres.find (movie->getGenre ()));
      if (g == genres.end ())
	 g = genres.begin ();
      row[colActors.genre] = g->second;
      row[colActors.editable] = false;
   }
   else {
      HActor actor (HActor::cast (entity)); Check3 (actor.isDefined ());
      row[colActors.name] = actor->getName ();
      row[colActors.year] = actor->getLifespan ();
      row[colActors.editable] = true;
   }
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortEntity (const Gtk::TreeModel::iterator& a,
			   const Gtk::TreeModel::iterator& b) {
   YGP::HEntity hEntity ((*a)[colActors.entry]);
   HMovie ha (HMovie::castDynamic (hEntity));
   if (ha) {
      hEntity = ((*b)[colActors.entry]);
      HMovie hb (HMovie::cast (hEntity)); Check3 (hb);

      int rc (Movie::removeIgnored (ha->getName ()).compare (Movie::removeIgnored (hb->getName ())));
      return rc ? rc : (ha->getName () < hb->getName ());
   }
   else {
      HActor ha (HActor::cast (hEntity)); Check3 (ha);
      hEntity = ((*b)[colActors.entry]);
      HActor hb (HActor::cast (hEntity)); Check3 (hb);

      int rc (Actor::removeIgnored (ha->getName ()).compare (Actor::removeIgnored (hb->getName ())));
      return rc ? rc : (ha->getName () < hb->getName ());
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
      YGP::HEntity hEntity (row[colActors.entry]);
      HActor actor (HActor::castDynamic (hEntity)); Check3 (actor.isDefined ());

      switch (column) {
      case 0:
	 if (value.size ()) {
	    Gtk::TreeModel::const_iterator i (findName (value));
	    if ((i != row) && (i != mOwnerObjects->children ().end ())) {
	       Glib::ustring e (_("Entry `%1' already exists!"));
	       e.replace (e.find ("%1"), 2, value);
	       throw (std::runtime_error (e));
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
Gtk::TreeIter ActorList::findEntity (const YGP::HEntity& entry, unsigned int level,
				     Gtk::TreeIter begin, Gtk::TreeIter end) const {
   while (begin != end) {
      Gtk::TreeModel::Row actRow (*begin);
      if (entry == actRow[colActors.entry])
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
