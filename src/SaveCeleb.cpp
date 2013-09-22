//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 11.03.2006
//COPYRIGHT   : Copyright (C) 2006, 2009 - 2011

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

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/liststore.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Storage.h"

#include "SaveCeleb.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent window
/// \param celeb Celebrity to match
/// \param celebs List of celebrities matching celeb
//-----------------------------------------------------------------------------
SaveCelebrity::SaveCelebrity (Gtk::Window& parent, const HCelebrity celeb, const std::vector<HCelebrity>& celebs)
   : Gtk::MessageDialog (parent, _("A celebrity with the same name already exists! Are they identic?"),
			 false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true),
     lstCelebs (NULL) {
   set_title (_("Choose matching celebrity"));
   Check1 (celeb);
   Check1 (celebs.size ());

   // Create string identifying celebrity to save
   Glib::ustring newCeleb (celeb->getName ());
   if (celeb->getBorn ().isDefined () || celeb->getDied ().isDefined ())
      newCeleb += " (" + celeb->getLifespan () + ") ";

   Gtk::Label* lblNewCeleb (new Gtk::Label (newCeleb));

   Glib::RefPtr <Gtk::ListStore> model (Gtk::ListStore::create (colCeleb));
   lstCelebs = new Gtk::TreeView (model);

   lstCelebs->append_column (_("Name"), colCeleb.name);
   lstCelebs->append_column (_("Born"), colCeleb.born);
   lstCelebs->append_column (_("Died"), colCeleb.died);

   set_response_sensitive (Gtk::RESPONSE_YES, false);
   lstCelebs->get_selection ()->signal_changed ().connect (mem_fun (*this, &SaveCelebrity::rowSelected));

   add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

   get_vbox ()->pack_start (*manage (lblNewCeleb), false, false, 5);
   get_vbox ()->pack_start (*manage (lstCelebs), false, false, 5);

   struct {
      const char*   table;
      Glib::ustring role;
   } roles[] =
      { { "Actors", _("actor") },
	{ "Interprets", _("interpret") },
	{ "Directors", _("director") } };

   // Fill table with matching celebrities
   for (std::vector<HCelebrity>::const_iterator i (celebs.begin ());
	i != celebs.end (); ++i) {
      Glib::ustring name ((*i)->getName ());

      Gtk::TreeRow row (*model->append ());
      row[colCeleb.id] = (*i)->getId ();
      row[colCeleb.born] = (*i)->getBorn ().toString ();
      row[colCeleb.died] = (*i)->getDied ().toString ();

      bool first (true);
      for (unsigned int r (0); r < (sizeof (roles) / sizeof (*roles)); ++r)
	 if (Storage::hasRole ((*i)->getId (), (roles[r]).table)) {
	    name += first ? " (" : ", ";
	    name += roles[r].role;
	    first = false;
	 }
      if (!first)
	 name += ')';

      row[colCeleb.name] = name;
   }

   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SaveCelebrity::~SaveCelebrity () {
}

//-----------------------------------------------------------------------------
/// Saves a celebrity in the passed role
/// \param celeb Celebrity to save
/// \param role Role celebrity should have
/// \throw
///   - DlgCanceled if the dialog was canceled
///   - std::exception Describing error
/// \remarks
///    - If the celebrity is unsaved (has no id), and the DB contains
///      a celebrity with the same name, it checks the DB-tables "Directors",
///      "Actors" and "Interpret" for the role of this celebrity
//-----------------------------------------------------------------------------
void SaveCelebrity::store (const HCelebrity celeb, const char* role,
			   Gtk::Widget& parent) throw (std::exception, DlgCanceled) {
   Check1 (celeb);
   TRACE8 ("SaveCelebrity::store (const HCelebrity, const char*, Gtk::Widget&) - " << celeb->getName ());

   if (celeb->getId ())
      Storage::updateCelebrity (celeb);
   else {
      std::vector<HCelebrity> celebs;
      Storage::getCelebrities (celeb->getName (), celebs);
      if (celebs.size ()) {
	 Check3 (parent.get_toplevel ());
	 SaveCelebrity dlg (*(Gtk::Window*)parent.get_toplevel (), celeb, celebs);
	 dlg.get_window ()->set_transient_for (parent.get_window ());
	 switch (dlg.run ()) {
	 case Gtk::RESPONSE_YES:
	    Check3 (dlg.getIdOfSelection ());

	    celeb->setId (dlg.getIdOfSelection ());
	    Storage::updateCelebrity (celeb);
	    Storage::setRole (celeb->getId (), role);
	    return;

	 case Gtk::RESPONSE_NO:
	    break;

	 default:
	    throw DlgCanceled ();
	 }
      }
      Storage::insertCelebrity (celeb, role);
   }
}

//-----------------------------------------------------------------------------
/// Creates the dialog
/// \param parent Parent window
/// \param celeb Celebrity to match
/// \param celebs List of celebrities matching celeb
/// \returns SaveCelebrity* Pointer to created dialog
/// \remarks Does not register a callback to free the created dialog!
//-----------------------------------------------------------------------------
SaveCelebrity* SaveCelebrity::create (Gtk::Window& parent, const HCelebrity celeb,
			   const std::vector<HCelebrity>& celebs) {
   SaveCelebrity* dlg (new SaveCelebrity (*(Gtk::Window*)parent.get_toplevel (), celeb, celebs));
   dlg->get_window ()->set_transient_for (parent.get_window ());
   return dlg;
}

//-----------------------------------------------------------------------------
/// Returns the ID of the selected celebrity
/// \returns unsigned long: ID of the selected celebrity
//-----------------------------------------------------------------------------
unsigned long SaveCelebrity::getIdOfSelection () {
   TRACE9 ("SaveCelebrity::getIdOfSelection ()");
   Check1 (lstCelebs);
   Check3 (lstCelebs->get_selection ());
   Check3 (lstCelebs->get_selection ()->get_selected ());

   Gtk::TreeRow row (*lstCelebs->get_selection ()->get_selected ());
   return row[colCeleb.id];
}

//-----------------------------------------------------------------------------
/// Callback after selecting a row: Enables/disables the YES-button
//-----------------------------------------------------------------------------
void SaveCelebrity::rowSelected () {
   set_response_sensitive (Gtk::RESPONSE_YES, lstCelebs->get_selection ()->get_selected ());
}
