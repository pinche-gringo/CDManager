#ifndef SAVECELEB_H
#define SAVECELEB_H

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


#include <vector>
#include <stdexcept>

#include "Celebrity.h"

#include <gtkmm/treeview.h>
#include <gtkmm/messagedialog.h>


/**Class to save a celebrity. Before storing the passed celebrity it
 * checks, if its name is unique and offers a dialog to connect the
 * celebrity with others (having the same name)
 */
class SaveCelebrity : public Gtk::MessageDialog {
 public:
   /**Dialog-canceled exception
    */
   class DlgCanceled : public std::runtime_error {
    public:
      explicit DlgCanceled () : std::runtime_error ("By user") { }
      virtual ~DlgCanceled () throw () { }
   };

   virtual ~SaveCelebrity ();

   static void store (const HCelebrity celeb, const char* role,
		      Gtk::Widget& parent) throw (std::exception, DlgCanceled);
   static SaveCelebrity* create (Gtk::Window& parent, const HCelebrity celeb, const std::vector<HCelebrity>& celebs);

   unsigned long getIdOfSelection ();

 protected:
   SaveCelebrity (Gtk::Window& parent, const HCelebrity celeb, const std::vector<HCelebrity>& celebs);

 private:
   //Prohibited manager functions
   SaveCelebrity (const SaveCelebrity& other);
   const SaveCelebrity& operator= (const SaveCelebrity& other);

   void rowSelected ();

   Gtk::TreeView* lstCelebs;

   /**Columns of the celebrity-list
    */
   class CelebColumns : public Gtk::TreeModel::ColumnRecord {
   public:
      CelebColumns () { add (name); add (born); add (died); add (id); }

      Gtk::TreeModelColumn<Glib::ustring> name;
      Gtk::TreeModelColumn<Glib::ustring> born;
      Gtk::TreeModelColumn<Glib::ustring> died;
      Gtk::TreeModelColumn<unsigned long> id;
   };
   CelebColumns colCeleb;
};

#endif
