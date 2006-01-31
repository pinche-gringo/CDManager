#ifndef NBPAGE_H
#define NBPAGE_H

//$Id: NBPage.h,v 1.3 2006/01/31 20:46:48 markus Exp $

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


#include <map>
#include <stack>
#include <vector>

#include <glibmm/ustring.h>

#include <gtkmm/action.h>
#include <gtkmm/treepath.h>
#include <gtkmm/actiongroup.h>

#include <YGP/Entity.h>

#include <sigc++/trackable.h>


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
   virtual void saveData () throw (Glib::ustring) = 0;
   virtual void getFocus () = 0;
   virtual void addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) = 0;
   virtual void removeMenu ();
   virtual void deleteSelection () = 0;
   virtual void undo () = 0;
   virtual void clear () = 0;
   virtual void export2HTML (unsigned int fd);

   bool isChanged () const { return false; }

 protected:
   NBPage (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave) : widget (NULL),
      menuSave (menuSave), statusbar (status), loaded (false) { }

   Gtk::Widget*              widget;
   Glib::RefPtr<Gtk::Action> menuSave;
   Gtk::Statusbar&           statusbar;

   typedef enum { NONE_SELECTED, OWNER_SELECTED, OBJECT_SELECTED } SELECTED;
   void enableEdit (SELECTED selected);
   void showStatus (const Glib::ustring& msgStatus);
   void enableSave (bool on = true) { menuSave->set_sensitive (on); }

   enum { NEW1, NEW2, NEW3, UNDO, DELETE, LAST };
   Glib::RefPtr<Gtk::Action> apMenus[LAST];

   /**Undo-info
    */
   class Undo {
   public:
      typedef enum { UNDEFINED = 0, INSERT, DELETE, CHANGED } CHGSPEC;
      Undo () { chgSpec.how = UNDEFINED; };
      Undo (CHGSPEC chg, unsigned int what, unsigned int col, Gtk::TreePath& row,
	    const Glib::ustring& value) : row (row), value (value) {
	 chgSpec.how = chg;
	 chgSpec.what = what;
	 chgSpec.column = col;
       }

      unsigned int how () const { return chgSpec.how; }
      unsigned int what () const { return chgSpec.what; }
      unsigned int column () const { return chgSpec.column; }
      const Gtk::TreePath& getPath () const { return row; }
      const Glib::ustring& getValue () const { return value; }

   private:
      struct {
	 unsigned int column : 16;
	 unsigned int how    : 2;
	 unsigned int what   : 3;
      } chgSpec;
      Gtk::TreePath row;
      Glib::ustring value;
   };

   bool loaded;
   std::stack<Undo>                     aUndo;
   std::vector<YGP::HEntity>            delEntries;
   std::map<YGP::HEntity, YGP::HEntity> delRelation;

 private:
   NBPage (const NBPage&);
   NBPage& operator= (const NBPage&);
};

#endif
