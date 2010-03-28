//$Id: OOList.cpp,v 1.28 2007/02/09 12:53:49 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : OwnerObjectList
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.28 $
//AUTHOR      : Markus Schwab
//CREATED     : 25.11.2004
//COPYRIGHT   : Copyright (C) 2004 - 2007, 2009, 2010

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
#include <YGP/CRegExp.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <XGP/XValue.h>

#include "OOList.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
OwnerObjectList::OwnerObjectList (const Genres& genres)
   : genres (genres), colOwnerObjects (NULL)
     , mGenres (Gtk::ListStore::create (colGenres)) {
   TRACE9 ("OwnerObjectList::OwnerObjectList (const Genres&)");
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
OwnerObjectList::~OwnerObjectList () {
   TRACE9 ("OwnerObjectList::~OwnerObjectList ()");
}


//-----------------------------------------------------------------------------
/// Initializes the class
/// \param cols: Columns of the model
//-----------------------------------------------------------------------------
void OwnerObjectList::init (const OwnerObjectColumns& cols) {
   TRACE9 ("OwnerObject::init ()");
   updateGenres ();

   colOwnerObjects = &cols;
   set_model (mOwnerObjects);

   append_column (getColumnName (), cols.name);
   append_column (_("Year"), cols.year);

   unsigned int index[] = { cols.name.index (), cols.year.index () };
   for (unsigned int i (0); i < (sizeof (index) / sizeof (*index)); ++i) {
      Gtk::TreeViewColumn* column (get_column (i));
      column->set_sort_column (index[i]);
      column->set_resizable ();

      Check3 (get_column_cell_renderer (i));
      Gtk::CellRenderer* r (get_column_cell_renderer (i)); Check3 (r);
      Check3 (typeid (*r) == typeid (Gtk::CellRendererText));
      Gtk::CellRendererText* rText (dynamic_cast<Gtk::CellRendererText*> (r));
      rText->property_editable () = true;
      rText->signal_edited ().connect
	 (bind (mem_fun (*this, &OwnerObjectList::valueChanged), i));
   }

   Gtk::CellRendererCombo* renderer (new Gtk::CellRendererCombo);
   renderer->property_text_column () = 0;
   renderer->property_model () = mGenres;
   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn
				(_("Genre"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text (), cols.genre);
   column->add_attribute (renderer->property_editable(), cols.chgAll);

   column->set_sort_column (cols.genre.index ());
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &OwnerObjectList::valueChanged), 2));

   mOwnerObjects->set_sort_func (cols.name,
				 sigc::mem_fun (*this, &OwnerObjectList::sortByName));
   mOwnerObjects->set_sort_func (cols.year,
				 sigc::mem_fun (*this, &OwnerObjectList::sortByYear));
   mOwnerObjects->set_sort_func (cols.genre,
				 sigc::mem_fun (*this, &OwnerObjectList::sortByGenre));
   set_headers_clickable ();
}

//-----------------------------------------------------------------------------
/// Appends an object to an owner in the list
/// \param object: Object to add
/// \param owner: Owner to add the object to
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::append (HEntity& object,
					     const Gtk::TreeModel::Row& owner) {
   TRACE3 ("OwnerObjectList::append (HEntity&, const Gtk::TreeModel::Row&)");
   Check2 (colOwnerObjects);
   Check1 (object);

   Gtk::TreeModel::Row newObj (*mOwnerObjects->append (owner.children ()));
   newObj[colOwnerObjects->entry] = object;
   return newObj;
}

//-----------------------------------------------------------------------------
/// Appends an owner to the list
/// \param owner: Owner to add
/// \param pos: Position in model for insert
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::insert (const HCelebrity& owner, const Gtk::TreeIter& pos) {
   TRACE3 ("OwnerObjectList::insert (const HCelebrity&, const Gtk::TreeIter&) - "
	   << (owner ? owner->getName ().c_str () : "None"));
   Check1 (owner);
   Check2 (colOwnerObjects);

   Gtk::TreeModel::Row newOwner (*mOwnerObjects->insert (pos));
   set (newOwner, owner);
   return newOwner;
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void OwnerObjectList::valueChanged (const Glib::ustring& path,
				    const Glib::ustring& value, unsigned int column) {
   TRACE9 ("OwnerObjectList::valueChanged (2x const Glib::ustring&, unsigned int) - "
	   << path << "->" << value);
   Check2 (column < 3);
   Check2 (colOwnerObjects);

   Gtk::TreeModel::Row row (*mOwnerObjects->get_iter (Gtk::TreeModel::Path (path)));
   Glib::ustring oldValue;

   try {
      if (row.parent ()) {
	 HEntity object (getObjectAt (row));

	 // First check, if value is valid
	 switch (column) {
	 case 0:
	    if (value.size ()) {
	       Gtk::TreeModel::const_iterator i (getObject (row.parent (), value));
	       if ((i != row) && (i != row.parent ()->children ().end ())) {
		  Glib::ustring e (_("Entry `%1' already exists!"));
		  e.replace (e.find ("%1"), 2, value);
		  throw YGP::InvalidValue (e);
	       }
	    }
	    oldValue = row[colOwnerObjects->name];
	    row[colOwnerObjects->name] = value;
	    setName (object, value);
	    break;

	 case 1:
	    setYear (object, value);
	    oldValue = row[colOwnerObjects->year];
	    row[colOwnerObjects->year] = value;
	    break;

	 case 2: {
	    oldValue = row[colOwnerObjects->genre];
	    oldValue = Glib::ustring (1, (char)(genres.getId (oldValue)));

	    int g (genres.getId (value));
	    if (g != -1) {
	       setGenre (object, g);
	       row[colOwnerObjects->genre] = value;
	       break;
	    }
	    else
	       throw YGP::InvalidValue (_("Unknown genre!"));
	    break; }
	 } // endswitch

	 if (value != oldValue)
	    signalObjectChanged.emit (row, column, oldValue);
      } // endif object edited
      else {
	 HCelebrity celeb (getCelebrityAt (row)); Check3 (celeb);

	 switch (column) {
	 case 0:
	    if (value.size ()) {
	       // Check if changes are valid
	       Gtk::TreeModel::const_iterator i (getOwner (value));
	       if ((i != row) && (i != mOwnerObjects->children ().end ())) {
		  Glib::ustring e (_("Entry `%1' already exists!"));
		  e.replace (e.find ("%1"), 2, value);
		  throw YGP::InvalidValue (e);
	       }
	    }
	    oldValue = row[colOwnerObjects->name];
	    celeb->setName (value);
	    row[colOwnerObjects->name] = celeb->getName ();
	    break;

	 case 1:
	    celeb->setLifespan (value);
	    oldValue = row[colOwnerObjects->year];
	    row[colOwnerObjects->year] = celeb->getLifespan ();
	    break;
	 } // end-switch

	 if ((value != oldValue) && (column < 2))
	    signalOwnerChanged.emit (row, column, oldValue);
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
void OwnerObjectList::updateGenres () {
   TRACE9 ("OwnerObjectList::updateGenres () - Genres: " << genres.size ());

   mGenres->clear ();
   for (unsigned int i (0); i < genres.size (); ++i) {
      Gtk::TreeModel::Row newGenre (*mGenres->append ());
      newGenre[colGenres.genre] = (genres.getGenre (i));
   }
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HEntity) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HEntity: Handle of the passed line
//-----------------------------------------------------------------------------
HEntity OwnerObjectList::getObjectAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   Check2 (colOwnerObjects);
   HEntity hEntity ((*iter)[colOwnerObjects->entry]); Check3 (hEntity);
   return hEntity;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HCelebrity) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HCelebrity: Handle of the selected line
//-----------------------------------------------------------------------------
HCelebrity OwnerObjectList::getCelebrityAt (const Gtk::TreeIter iter) const {
   TRACE9 ("GetCelibrity: " << (*iter)[colOwnerObjects->name]);
   Check2 (!(*iter)->parent ());
   Check2 (colOwnerObjects);
   HCelebrity owner (boost::dynamic_pointer_cast<Celebrity> ((HEntity)(*iter)[colOwnerObjects->entry]));
   Check3 (owner);
   TRACE7 ("CDManager::getCelebrityAt (const Gtk::TreeIter&) - Selected: " <<
	   owner->getId () << '/' << owner->getName ());
   return owner;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setName (HEntity& object, const Glib::ustring& value) {
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setYear (HEntity& object, const Glib::ustring& value) throw (std::exception) {
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setGenre (HEntity& object, unsigned int value) {
}

//-----------------------------------------------------------------------------
/// Sets the genre in the passed row
/// \param row: Row to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void OwnerObjectList::changeGenre (Gtk::TreeModel::Row& row, unsigned int value) {
   TRACE9 ("OwnerObjectList::changeGenre (Gtk::TreeModel::Row&, unsigned int) - " << value);
   Check2 (colOwnerObjects);

   if (value >= genres.size ())
      value = 0;
   row[colOwnerObjects->genre] = genres.getGenre (value);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring first names,
/// articles, ...)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByName (const Gtk::TreeModel::iterator& a,
				 const Gtk::TreeModel::iterator& b) const {
   Check2 (a->parent () == b->parent ());
   if (a->parent ())
      return sortEntity (a, b);
   else
      return sortOwner (a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the year
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByYear (const Gtk::TreeModel::iterator& a,
				 const Gtk::TreeModel::iterator& b) const {
   Check2 (a->parent () == b->parent ());
   if (a->parent ()) {
      Gtk::TreeRow ra (*a);
      Gtk::TreeRow rb (*b);

      Glib::ustring sa (ra[colOwnerObjects->year]);
      Glib::ustring sb (rb[colOwnerObjects->year]);
      YGP::AYear ya (sa);
      YGP::AYear yb (sb);
      return ya.compare (yb);
   }
   else
      return sortOwner (a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the genre
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByGenre (const Gtk::TreeModel::iterator& a,
				  const Gtk::TreeModel::iterator& b) const {
   if ((*a)->parent ()) {
      Gtk::TreeRow ra (*a);
      Gtk::TreeRow rb (*b); Check2 (rb->parent ());

      Glib::ustring sa (ra[colOwnerObjects->genre]);
      Glib::ustring sb (rb[colOwnerObjects->genre]);
      return sa.compare (sb);
   }
   else
      return sortOwner (a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the owner in the listbox according to the name
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortOwner (const Gtk::TreeModel::iterator& a,
				const Gtk::TreeModel::iterator& b) const {
   Check2 (!a->parent ()); Check2 (!b->parent ());
   HCelebrity ha (getCelebrityAt (a)); Check3 (ha);
   HCelebrity hb (getCelebrityAt (b)); Check3 (hb);

   TRACE9 ("OwnerObjectList::sortOwner (2x const Gtk::TreeModel::iterator&) - " << ha->getName () << "<->" << hb->getName ());
   int rc (Celebrity::removeIgnored (ha->getName ()).compare (Celebrity::removeIgnored (hb->getName ())));
   return rc ? rc : (ha->getName () < hb->getName ());

}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortEntity (const Gtk::TreeModel::iterator& a,
				 const Gtk::TreeModel::iterator& b) const {
   Check2 (a->parent ()); Check2 (b->parent ());
   Check2 (colOwnerObjects);
   Gtk::TreeRow ra (*a);
   Gtk::TreeRow rb (*b); Check2 (rb->parent ());

   Glib::ustring sa (ra[colOwnerObjects->name]);
   Glib::ustring sb (rb[colOwnerObjects->name]);
   TRACE9 ("OwnerObjectList::sortEntity (2x const Gtk::TreeModel::iterator&) - "
	   << sa << '/' << sb << '=' << sa.compare (sb));
   return sa.compare (sb);
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the owner having the passed value as name
/// \param name: Name of entry
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getOwner (const Glib::ustring& name) const {
   Check2 (colOwnerObjects);

   for (Gtk::TreeModel::const_iterator i (mOwnerObjects->children ().begin ());
	i != mOwnerObjects->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (actRow[colOwnerObjects->name] == name)
	 return i;
   }
   return mOwnerObjects->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the owner identified by the passed handle
/// \param owner: Handle to owner
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getOwner (const HCelebrity& owner) const {
   Check2 (colOwnerObjects);
   Check2 (owner);

   for (Gtk::TreeModel::const_iterator i (mOwnerObjects->children ().begin ());
	i != mOwnerObjects->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (owner == (HEntity)actRow[colOwnerObjects->entry])
	 return i;
   }
   return mOwnerObjects->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children having the passed value as name
/// \param parent: Parent row
/// \param name: Name of entry
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject (const Gtk::TreeIter& parent,
						     const Glib::ustring& name) const {
   Check2 (colOwnerObjects);

   for (Gtk::TreeModel::const_iterator i (parent->children ().begin ());
	i != parent->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (actRow[colOwnerObjects->name] == name)
	 return i;
   }
   return parent->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children identified by the passed object
/// \param parent: Parent row
/// \param object: Entry to find
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject (const Gtk::TreeIter& parent,
						     const HEntity& object) const {
   Check2 (colOwnerObjects);

   for (Gtk::TreeModel::const_iterator i (parent->children ().begin ());
	i != parent->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (object == (HEntity)actRow[colOwnerObjects->entry])
	 return i;
   }
   return parent->children ().end ();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children identified by the passed object
/// \param object: Entry to find
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject (const HEntity& object) const {
   Check2 (colOwnerObjects);

   for (Gtk::TreeModel::const_iterator i (mOwnerObjects->children ().begin ());
	i != mOwnerObjects->children ().end (); ++i)
      for (Gtk::TreeModel::const_iterator j (i->children ().begin ());
	   j != i->children ().end (); ++j) {
	 Gtk::TreeModel::Row actRow (*j);
	 if (object == (HEntity)actRow[colOwnerObjects->entry])
	    return j;
   }
   return mOwnerObjects->children ().end ();
}

//-----------------------------------------------------------------------------
/// Selects the passed row (as only one) and centers it
/// \param i: Iterator to row to select
//-----------------------------------------------------------------------------
void OwnerObjectList::selectRow (const Gtk::TreeModel::const_iterator& i) {
   Glib::RefPtr<Gtk::TreeSelection> sel (get_selection ());
   Gtk::TreePath path (mOwnerObjects->get_path (i));
   scroll_to_row (path, 0.5);
   set_cursor (path);
   sel->select (path);
}

//-----------------------------------------------------------------------------
/// Updates the displayed movies, showing them in the passed language
/// \param row: Row to update
/// \param obj: Object, whose values to set
//-----------------------------------------------------------------------------
void OwnerObjectList::set (Gtk::TreeModel::Row& row, const HEntity& obj) {
   row[colOwnerObjects->entry] = obj;
   update (row);
}

//-----------------------------------------------------------------------------
/// Updates the displayed movies, showing them in the passed language
/// \param row: Row to update
//-----------------------------------------------------------------------------
void OwnerObjectList::update (Gtk::TreeModel::Row& row) {
   if (row->parent ())
      row[colOwnerObjects->chgAll] = true;
   else {
      HCelebrity owner (getCelebrityAt (row));
      row[colOwnerObjects->name] = owner->getName ();
      row[colOwnerObjects->year] = owner->getLifespan ();
      row[colOwnerObjects->chgAll] = false;
   }
}
