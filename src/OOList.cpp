//$Id: OOList.cpp,v 1.1 2004/11/26 03:31:54 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : OwnerObjectList
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
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

#define CHECK 9
#define TRACELEVEL 9
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

   set_model (mOwnerObjects);

   append_column (_("TODO"), colOwnerObjects.name);
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
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
OwnerObjectList::~OwnerObjectList () {
   TRACE9 ("OwnerObjectList::~OwnerObjectList ()");
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
	   << (owner.isDefined () ? owner->name.c_str () : "None"));
   Check1 (owner.isDefined ());

   Gtk::TreeModel::Row newOwner (*mOwnerObjects->append ());
   newOwner[colOwnerObjects.entry] = YGP::Handle<YGP::Entity>::cast (owner);
   newOwner[colOwnerObjects.name] = owner->name;
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
	 case 0:
	    setName (object, value);
	    row[colOwnerObjects.name] = value;
	    break;
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
	 case 0:
	    row[colOwnerObjects.name] = celeb->name = value;
	    break;
	 case 1:
	    if (value.size ()) {
	       unsigned int pos (value.find ("- "));
	       if (pos != std::string::npos)
		  celeb->died = value.substr (pos + 2);
	       else
		  celeb->died.undefine ();

	       if ((pos == std::string::npos)
		   || ((pos > 0) && (value[pos - 1] == ' ')))
		  celeb->born = value.substr (0, pos - 1);
	       else
		  celeb->born.undefine ();
	    }
	    else {
	       celeb->born.undefine ();
	       celeb->died.undefine ();
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
	   owner->id << '/' << owner->name);
   return owner;
}


//-----------------------------------------------------------------------------
/// Shows the time of live of the passed director
/// \param director: Director to display
/// \returns Glib::ustring: Text to display
//-----------------------------------------------------------------------------
Glib::ustring OwnerObjectList::getLiveSpan (const HCelebrity& owner) {
   TRACE9 ("OwnerObjectList::getLiveSpan (const HCelebrity&) - "
	   << (owner.isDefined () ? owner->name.c_str () : "None"));
   Check1 (owner.isDefined ());

   Glib::ustring tmp (owner->born.toString ());
   if (owner->born.isDefined ())
      tmp.append (1, ' ');
   if (owner->died.isDefined ())
      tmp.append ("- ");
   tmp.append (owner->died.toString ());
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
