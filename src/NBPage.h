#ifndef NBPAGE_H
#define NBPAGE_H

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


#include <map>
#include <stack>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include <glibmm/ustring.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <gtkmm/treepath.h>
#include <gtkmm/shortcutcontroller.h>

#include <YGP/Entity.h>

#include <sigc++/trackable.h>


typedef boost::shared_ptr<YGP::Entity> HEntity;


// Forward declarations
namespace Gtk {
   class Widget;
   class Statusbar;
}


/**Baseclass for notebook-pages
 */
class NBPage : public sigc::trackable {
 public:
   virtual ~NBPage ();

   virtual Gtk::Widget* getWindow () const { return widget; }

   bool isLoaded () const { return loaded; }

   virtual void loadData () = 0;
   virtual void saveData () = 0;
   virtual void getFocus () = 0;
   /// Adds the page-related menu-entries to menuEdit (the Edit-menu) and
   /// optional top-level menus to menuOther. The according actions are
   /// added to grpAction (which is inserted as "page" into the window), the
   /// keyboard shortcuts to shortcuts.
   virtual void addMenu (Glib::RefPtr<Gio::Menu> menuEdit, Glib::RefPtr<Gio::Menu> menuOther,
			 Glib::RefPtr<Gio::SimpleActionGroup> grpAction,
			 Glib::RefPtr<Gtk::ShortcutController> shortcuts) = 0;
   virtual void removeMenu ();
   virtual void deleteSelection () = 0;
   virtual void undo () = 0;
   virtual void clear ();
   virtual void export2HTML (unsigned int fd, const std::string& lang);

   bool isChanged () const { return !aUndo.empty (); }

   NBPage (const NBPage&) = delete;
   NBPage& operator= (const NBPage&) = delete;

   static void addMenuEntry (const Glib::RefPtr<Gio::Menu>& menu, const Glib::ustring& label,
			     const Glib::ustring& action, const Glib::ustring& accel = Glib::ustring (),
			     const Glib::RefPtr<Gtk::ShortcutController>& shortcuts = {});

 protected:
   NBPage (Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave) : widget (nullptr),
      menuSave (menuSave), statusbar (status), loaded (false) { }

   Gtk::Widget*                    widget;
   Glib::RefPtr<Gio::SimpleAction> menuSave;
   Gtk::Statusbar&                 statusbar;

   typedef enum { NONE_SELECTED, OWNER_SELECTED, OBJECT_SELECTED } SELECTED;
   void enableEdit (SELECTED selected);
   void showStatus (const Glib::ustring& msgStatus);
   void showError (const Glib::ustring& msg, const Glib::ustring& title = Glib::ustring ());
   void addStatusWidget (Gtk::Widget& widget);
   void removeStatusWidget (Gtk::Widget& widget);
   void enableSave (bool on = true) { menuSave->set_enabled (on); }

   enum { NEW1, NEW2, NEW3, UNDO, DELETE, LAST };
   Glib::RefPtr<Gio::SimpleAction> apMenus[LAST];

   /**Undo-info
    */
   class Undo {
   public:
      typedef enum { UNDEFINED = 0, INSERT, DELETE, CHANGED } CHGSPEC;
      Undo () { chgSpec.how = UNDEFINED; };
      Undo (CHGSPEC chg, unsigned int what, unsigned int col, HEntity entity,
	    const Gtk::TreePath& row, const Glib::ustring& value);

      unsigned int how () const { return chgSpec.how; }
      unsigned int what () const { return chgSpec.what; }
      unsigned int column () const { return chgSpec.column; }

      const HEntity getEntity () const { return entity; }
      const Gtk::TreePath& getPath () const { return row; }
      const Glib::ustring& getValue () const { return value; }

   private:
      struct {
	 unsigned int column : 16;
	 unsigned int how    : 2;
	 unsigned int what   : 3;
      } chgSpec;
      HEntity  entity;
      Gtk::TreePath row;
      Glib::ustring value;
   };

   bool loaded;
   std::stack<Undo>           aUndo;
   std::map<HEntity, HEntity> delRelation;
};

#endif
