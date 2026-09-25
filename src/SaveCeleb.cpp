//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 11.03.2006
//COPYRIGHT   : Copyright (C) 2006, 2009 - 2011, 2026

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

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/label.h>

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
			 false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO, true),
     lstCelebs (nullptr) {
   set_title (_("Choose matching celebrity"));
   Check1 (celeb);
   Check1 (celebs.size ());

   // Create string identifying celebrity to save
   Glib::ustring newCeleb (celeb->getName ());
   if (celeb->getBorn ().isDefined () || celeb->getDied ().isDefined ())
      newCeleb += " (" + celeb->getLifespan () + ") ";

   Gtk::Label* lblNewCeleb (Gtk::make_managed<Gtk::Label> (newCeleb));
   lblNewCeleb->set_margin (5);

   Glib::RefPtr <Gtk::ListStore> model (Gtk::ListStore::create (colCeleb));
   lstCelebs = Gtk::make_managed<Gtk::TreeView> (model);
   lstCelebs->set_margin (5);

   lstCelebs->append_column (_("Name"), colCeleb.name);
   lstCelebs->append_column (_("Born"), colCeleb.born);
   lstCelebs->append_column (_("Died"), colCeleb.died);

   set_response_sensitive (Gtk::ResponseType::YES, false);
   lstCelebs->get_selection ()->signal_changed ().connect (sigc::mem_fun (*this, &SaveCelebrity::rowSelected));

   add_button (_("_Cancel"), Gtk::ResponseType::CANCEL);

   get_content_area ()->append (*lblNewCeleb);
   get_content_area ()->append (*lstCelebs);

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
void SaveCelebrity::store (const HCelebrity celeb, const char* role, Gtk::Widget& parent) {
   Check1 (celeb);
   TRACE8 ("SaveCelebrity::store (const HCelebrity, const char*, Gtk::Widget&) - " << celeb->getName ());

   if (celeb->getId ())
      Storage::updateCelebrity (celeb);
   else {
      std::vector<HCelebrity> celebs;
      Storage::getCelebrities (celeb->getName (), celebs);
      if (celebs.size ()) {
	 Gtk::Window* win (dynamic_cast<Gtk::Window*> (parent.get_root ()));
	 Check3 (win);
	 SaveCelebrity dlg (*win, celeb, celebs);
	 switch (dlg.run ()) {
	 case Gtk::ResponseType::YES:
	    Check3 (dlg.getIdOfSelection ());

	    celeb->setId (dlg.getIdOfSelection ());
	    Storage::updateCelebrity (celeb);
	    Storage::setRole (celeb->getId (), role);
	    return;

	 case Gtk::ResponseType::NO:
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
   return new SaveCelebrity (parent, celeb, celebs);
}

//-----------------------------------------------------------------------------
/// Shows the dialog modally and waits until the user responds (replacement
/// for Gtk::Dialog::run, which doesn't exist anymore in GTKMM-4)
/// \returns int Response of the user (Gtk::ResponseType::DELETE_EVENT, if
///          the dialog was closed)
//-----------------------------------------------------------------------------
int SaveCelebrity::run () {
   TRACE9 ("SaveCelebrity::run ()");

   int response (Gtk::ResponseType::NONE);
   Glib::RefPtr<Glib::MainLoop> loop (Glib::MainLoop::create ());
   sigc::connection conn (signal_response ().connect ([&response, loop] (int id) {
	    response = id;
	    loop->quit ();
	 }));

   show ();
   loop->run ();
   conn.disconnect ();
   hide ();
   return response;
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
   set_response_sensitive (Gtk::ResponseType::YES, bool (lstCelebs->get_selection ()->get_selected ()));
}
