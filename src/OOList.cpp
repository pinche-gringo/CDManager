//$Id: OOList.cpp,v 1.6 2004/12/04 04:05:21 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : OwnerObjectList
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.6 $
//AUTHOR      : Markus Schwab
//CREATED     : 25.11.2004
//COPYRIGHT   : Copyright (A) 2004

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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <XGP/XValue.h>

#include "RendererList.h"

#include "OOList.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
OwnerObjectList::OwnerObjectList (const std::map<unsigned int, Glib::ustring>& genres)
   : genres (genres), mOwnerObjects (Gtk::TreeStore::create (colOwnerObjects)) {
   TRACE9 ("OwnerObjectList::OwnerObjectList (const std::map<unsigned int, Glib::ustring>&)");
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
OwnerObjectList::~OwnerObjectList () {
   TRACE9 ("OwnerObjectList::~OwnerObjectList ()");
}


//-----------------------------------------------------------------------------
/// Initializes the class
//-----------------------------------------------------------------------------
void OwnerObjectList::init () {
   TRACE9 ("OwnerObject::init ()");
   set_model (mOwnerObjects);

   append_column (getColumnName (), colOwnerObjects.name);
   append_column (_("Year"), colOwnerObjects.year);

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
	 (bind (mem_fun (*this, &OwnerObjectList::valueChanged), i));
   }

   CellRendererList* const renderer (new CellRendererList ());
   renderer->property_editable () = true;
   Gtk::TreeViewColumn* const column (new Gtk::TreeViewColumn
				      (_("Genre"), *Gtk::manage (renderer)));
   append_column (*Gtk::manage (column));
   column->add_attribute (renderer->property_text(), colOwnerObjects.genre);

   column->set_sort_column (3);
   column->set_resizable ();

   renderer->signal_edited ().connect
      (bind (mem_fun (*this, &OwnerObjectList::valueChanged), 2));

   mOwnerObjects->set_sort_func (colOwnerObjects.name,
				 sigc::mem_fun (*this, &OwnerObjectList::sortByName));
}

//-----------------------------------------------------------------------------
/// Appends an object to an owner in the list
/// \param object: Object to add
/// \param owner: Owner to add the object to
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::append (HEntity& object,
					     const Gtk::TreeModel::Row& owner) {
   TRACE3 ("OwnerObjectList::append (HEntity&, const Gtk::TreeModel::Row)");
   Check1 (object.isDefined ());

   Gtk::TreeModel::Row newObj (*mOwnerObjects->append (owner.children ()));
   newObj[colOwnerObjects.entry] = object;
   return newObj;
}

//-----------------------------------------------------------------------------
/// Appends an owner to the list
/// \param owner: Owner to add
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::append (const HCelebrity& owner) {
   TRACE3 ("OwnerObjectList::append (const HCelebrity&) - "
	   << (owner.isDefined () ? owner->getName ().c_str () : "None"));
   Check1 (owner.isDefined ());

   Gtk::TreeModel::Row newOwner (*mOwnerObjects->append ());
   newOwner[colOwnerObjects.entry] = YGP::Handle<YGP::Entity>::cast (owner);
   newOwner[colOwnerObjects.name] = owner->getName ();
   newOwner[colOwnerObjects.year] = getLiveSpan (owner);

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

   Gtk::TreeModel::Row row (*mOwnerObjects->get_iter (Gtk::TreeModel::Path (path)));

   try {
      if (row.parent ()) {
	 HEntity object (getObjectAt (row));
	 signalObjectChanged.emit (object);
	 switch (column) {
	 case 0: {
	    Gtk::TreeModel::const_iterator i (getObject (row.parent (), value));
	    if ((i != row) && (i != row.parent ()->children ().end ())) {
	       Glib::ustring e (_("Entry `%1' already exists!"));
	       e.replace (e.find ("%1"), 2, value);
	       throw (std::runtime_error (e));
	    }
	    setName (object, value);
	    row[colOwnerObjects.name] = value;
	    break; }

	 case 1:
	    setYear (object, value);
	    row[colOwnerObjects.year] = value;
	    break;

	 case 2: {
	    for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
		 g != genres.end (); ++g)
	       if (g->second == value) {
		  setGenre (object, g->first);
		  row[colOwnerObjects.genre] = value;
		  signalObjectGenreChanged.emit (object);
		  return;
	       }
	    throw (std::invalid_argument (_("Unknown genre!")));
	    break; }
	 } // endswitch
      } // endif object edited
      else {
	 HCelebrity celeb (getCelebrityAt (row));
	 Check3 (celeb.isDefined ());
	 signalOwnerChanged.emit (celeb);

	 switch (column) {
	 case 0: {
	    Gtk::TreeModel::const_iterator i (getOwner (value));
	    if ((i != row) && (i != mOwnerObjects->children ().end ())) {
		  Glib::ustring e (_("Entry `%1' already exists!"));
		  e.replace (e.find ("%1"), 2, value);
		  throw (std::runtime_error (e));
	    }
	    celeb->setName (value);
	    row[colOwnerObjects.name] = celeb->getName ();
	    break; }

	 case 1:
	    if (value.size ()) {
	       unsigned int pos (value.find ("- "));
	       if (pos != std::string::npos)
		  celeb->setDied (value.substr (pos + 2));
	       else
		  celeb->undefineDied ();

	       if ((pos == std::string::npos)
		   || ((pos > 0) && (value[pos - 1] == ' ')))
		  celeb->setBorn (value.substr (0, pos - 1));
	       else
		  celeb->undefineBorn ();
	    }
	    else {
	       celeb->undefineDied ();
	       celeb->undefineBorn ();
	    }

	    row[colOwnerObjects.year] = getLiveSpan (celeb);
	    break;
	 } // end-switch
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

   Check3 (get_column_cell_renderer (2));
   Gtk::CellRenderer* r (get_column_cell_renderer (2)); Check3 (r);
   Check3 (typeid (*r) == typeid (CellRendererList));
   CellRendererList* renderer (dynamic_cast<CellRendererList*> (r));

   for (std::map<unsigned int, Glib::ustring>::const_iterator g (genres.begin ());
	g != genres.end (); ++g)
      renderer->append_list_item (g->second);
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HEntity) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HEntity: Handle of the passed line
//-----------------------------------------------------------------------------
HEntity OwnerObjectList::getObjectAt (const Gtk::TreeIter iter) const {
   Check2 ((*iter)->parent ());
   HEntity hRec ((*iter)[colOwnerObjects.entry]); Check3 (hRec.isDefined ());
   return hRec;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HCelebrity) at the passed position
/// \param iter: Iterator to position in the list
/// \returns HCelebrity: Handle of the selected line
//-----------------------------------------------------------------------------
HCelebrity OwnerObjectList::getCelebrityAt (const Gtk::TreeIter iter) const {
   Check2 (!(*iter)->parent ());
   HEntity hObj ((*iter)[colOwnerObjects.entry]); Check3 (hObj.isDefined ());
   HCelebrity owner (HCelebrity::cast (hObj));
   TRACE7 ("CDManager::getCelebrityAt (const Gtk::TreeIter&) - Selected: " <<
	   owner->getId () << '/' << owner->getName ());
   return owner;
}


//-----------------------------------------------------------------------------
/// Shows the time of live of the passed director
/// \param director: Director to display
/// \returns Glib::ustring: Text to display
//-----------------------------------------------------------------------------
Glib::ustring OwnerObjectList::getLiveSpan (const HCelebrity& owner) {
   TRACE9 ("OwnerObjectList::getLiveSpan (const HCelebrity&) - "
	   << (owner.isDefined () ? owner->getName ().c_str () : "None"));
   Check1 (owner.isDefined ());

   Glib::ustring tmp (owner->getBorn ().toString ());
   if (owner->getBorn ().isDefined ())
      tmp.append (1, ' ');
   if (owner->getDied ().isDefined ()) {
      tmp.append ("- ");
      tmp.append (owner->getDied ().toString ());
   }
   return tmp;
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
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setYear (HEntity& object, const Glib::ustring& value) {
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

   std::map<unsigned int, Glib::ustring>::const_iterator g
      (genres.find (value));
   if (g == genres.end ())
      g = genres.begin ();
   row[colOwnerObjects.genre] = g->second;
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring first names,
/// articles, ...)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByName (const Gtk::TreeModel::iterator& a,
				 const Gtk::TreeModel::iterator& b) {
   if ((*a)->parent ())
      return sortEntity (a, b);
   else {
      HCelebrity ha (getCelebrityAt (a)); Check3 (ha.isDefined ());
      HCelebrity hb (getCelebrityAt (b)); Check3 (hb.isDefined ());

      Glib::ustring aname (Celebrity::removeIgnored (ha->getName ()));
      Glib::ustring bname (Celebrity::removeIgnored (hb->getName ()));

      TRACE9 ("OwnerObjectList::sortByName (2x const Gtk::TreeModel::iterator&) - "
	      << aname << '/' << bname << '='
	      << ((aname < bname) ? -1 : (bname < aname) ? 1
		  : ha->getName ().compare (hb->getName ())));
      return ((aname < bname) ? -1 : (bname < aname) ? 1
	      : ha->getName ().compare (hb->getName ()));
   }
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortEntity (const Gtk::TreeModel::iterator& a,
				 const Gtk::TreeModel::iterator& b) {
   Gtk::TreeRow ra (*a);
   Gtk::TreeRow rb (*b); Check2 (rb->parent ());

   Glib::ustring sa (ra[colOwnerObjects.name]);
   Glib::ustring sb (rb[colOwnerObjects.name]);
   TRACE9 ("OwnerObjectList::sortEntity (2x const Gtk::TreeModel::iterator&) - "
	   << sa << '/' << sb << '=' << sa.compare (sb));
   return sa.compare (sb);
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the owner having the passed value as name
/// \param name: Name of entry
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getOwner (const Glib::ustring& name) {
   for (Gtk::TreeModel::const_iterator i (mOwnerObjects->children ().begin ());
	i != mOwnerObjects->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (actRow[colOwnerObjects.name] == name)
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
						     const Glib::ustring& name) {
   for (Gtk::TreeModel::const_iterator i (parent->children ().begin ());
	i != parent->children ().end (); ++i) {
      Gtk::TreeModel::Row actRow (*i);
      if (actRow[colOwnerObjects.name] == name)
	 return i;
   }
   return parent->children ().end ();
}
