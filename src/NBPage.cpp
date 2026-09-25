//PROJECT     : CDManager
//SUBSYSTEM   : NBPage
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 20.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009, 2010, 2026

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


#include <memory>

#include <giomm/action.h>
#include <giomm/menuitem.h>

#include <gtkmm/box.h>
#include <gtkmm/shortcut.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/shortcutaction.h>
#include <gtkmm/shortcuttrigger.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <XGP/XDialog.h>

#include "NBPage.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
NBPage::~NBPage () {
}


//-----------------------------------------------------------------------------
/// Enables or disables the edit-menus entries according to the selection
/// \param selected: Kind of the currently selected entry
//-----------------------------------------------------------------------------
void NBPage::enableEdit (SELECTED selected) {
   TRACE9 ("NBPage::enableEdit (SELECTED) - " << selected);
   Check2 (apMenus[NEW1]); Check2 (apMenus[NEW2]);

   Check2 (apMenus[DELETE]);

   apMenus[DELETE]->set_enabled (selected != NONE_SELECTED);
   apMenus[NEW1]->set_enabled (true);
   apMenus[NEW2]->set_enabled (selected > NONE_SELECTED);
   if (apMenus[NEW3])
      apMenus[NEW3]->set_enabled (selected == OBJECT_SELECTED);
}

//-----------------------------------------------------------------------------
/// Changes the text of the status-line
/// \param msgStatus: Text to display in the status-line
//-----------------------------------------------------------------------------
void NBPage::showStatus (const Glib::ustring& msgStatus) {
   statusbar.pop ();
   statusbar.push (msgStatus);
}

//-----------------------------------------------------------------------------
/// Shows a (modal) error message, transient for the window of the page
/// \param msg: Message to display
/// \param title: Title of the dialog; may be empty
//-----------------------------------------------------------------------------
void NBPage::showError (const Glib::ustring& msg, const Glib::ustring& title) {
   Gtk::Window* win (widget ? dynamic_cast<Gtk::Window*> (widget->get_root ()) : nullptr);
   std::unique_ptr<Gtk::MessageDialog> dlg
      (win ? new Gtk::MessageDialog (*win, msg, false, Gtk::MessageType::ERROR)
       : new Gtk::MessageDialog (msg, false, Gtk::MessageType::ERROR));
   if (title.size ())
      dlg->set_title (title);
   XGP::runModal (*dlg);
}

//-----------------------------------------------------------------------------
/// Adds a widget to the right of the statusbar
/// \param widget: Widget to add
/// \remarks The statusbar must be inside a (horizontal) box
//-----------------------------------------------------------------------------
void NBPage::addStatusWidget (Gtk::Widget& widget) {
   Gtk::Box* box (dynamic_cast<Gtk::Box*> (statusbar.get_parent ())); Check3 (box);
   box->append (widget);
}

//-----------------------------------------------------------------------------
/// Removes a widget previously added with addStatusWidget
/// \param widget: Widget to remove
//-----------------------------------------------------------------------------
void NBPage::removeStatusWidget (Gtk::Widget& widget) {
   Gtk::Box* box (dynamic_cast<Gtk::Box*> (statusbar.get_parent ())); Check3 (box);
   box->remove (widget);
}

//-----------------------------------------------------------------------------
/// Appends an entry to a menu; if an accelerator is passed, it's displayed in
/// the menu and registered as shortcut
/// \param menu: Menu to append the entry to
/// \param label: Label of the entry
/// \param action: Detailed name of the action (like "page.FUndo" or
///    "page.View::ByFilm")
/// \param accel: Accelerator (like "<ctl>Z"); may be empty
/// \param shortcuts: Controller to add the shortcut to
//-----------------------------------------------------------------------------
void NBPage::addMenuEntry (const Glib::RefPtr<Gio::Menu>& menu, const Glib::ustring& label,
			   const Glib::ustring& action, const Glib::ustring& accel,
			   const Glib::RefPtr<Gtk::ShortcutController>& shortcuts) {
   TRACE9 ("NBPage::addMenuEntry (...) - " << action << " - " << accel);
   Check1 (menu);

   Glib::RefPtr<Gio::MenuItem> item (Gio::MenuItem::create (label, action));
   if (accel.size () && shortcuts) {
      Glib::RefPtr<Gtk::ShortcutTrigger> trigger (Gtk::ShortcutTrigger::parse_string (accel));
      if (trigger) {
	 item->set_attribute_value ("accel", Glib::Variant<Glib::ustring>::create (accel));

	 Glib::ustring name;
	 Glib::VariantBase target;
	 Gio::Action::parse_detailed_name_variant (action, name, target);
	 Glib::RefPtr<Gtk::Shortcut> shortcut (Gtk::Shortcut::create (trigger, Gtk::NamedAction::create (name)));
	 if (target)
	    shortcut->set_arguments (target);
	 shortcuts->add_shortcut (shortcut);
      }
      else {
	 TRACE1 ("NBPage::addMenuEntry (...) - Invalid accelerator " << accel);
      }
   }
   menu->append_item (item);
}

//-----------------------------------------------------------------------------
/// Removes any created page-related menus
//-----------------------------------------------------------------------------
void NBPage::removeMenu () {
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
/// \param lang: Language, in which to export
//-----------------------------------------------------------------------------
void NBPage::export2HTML (unsigned int, const std::string&) {
}


//-----------------------------------------------------------------------------
/// Constructor of the undo-information
/// \param chg: Flag, how information was changed
/// \param what: Specifies what kind of entity was changed
/// \param col: Specifies what in the entity was changed
/// \param entity: Changed entity
/// \param row: Listbox-line related to the changed entity
/// \param value: Old value of changed entry
//-----------------------------------------------------------------------------
NBPage::Undo::Undo (CHGSPEC chg, unsigned int what, unsigned int col, HEntity entity,
		    const Gtk::TreePath& row, const Glib::ustring& value)
   : entity (entity), row (row), value (value) {
   TRACE9 ("NBPage::Undo::Undo (...)");
   chgSpec.how = chg;
   chgSpec.what = what;
   chgSpec.column = col;
}

//-----------------------------------------------------------------------------
/// Resets the loaded-flag
//-----------------------------------------------------------------------------
void NBPage::clear () {
   loaded = false;
}
