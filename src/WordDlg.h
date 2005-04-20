#ifndef WORDDLG_H
#define WORDDLG_H

//$Id: WordDlg.h,v 1.1 2005/04/20 05:42:39 markus Rel $

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


#include <gtkmm/table.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>


namespace Gtk {
   class Entry;
   class Button;
};


/**Table permitting to manipulate the words.
 */
class WordDialog : public Gtk::Table {
 public:
   static Gtk::Widget* makeDialog ();
   static void commitDialogData (Gtk::Widget* dialog);

 protected:
   WordDialog ();
   virtual ~WordDialog ();

   void commit ();

   class WordColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      WordColumns () { add (word); }

      Gtk::TreeModelColumn<Glib::ustring> word;
   };

   Gtk::TreeModel::Row appendWord (Glib::RefPtr<Gtk::ListStore>& list,
				   const Glib::ustring& value);
   void entryChanged (unsigned int which);
   void entrySelected (unsigned int which);
   void onAdd (unsigned int which);
   void onDelete (unsigned int which);

 private:
   WordColumns colWords;
   Glib::RefPtr<Gtk::ListStore> names;
   Glib::RefPtr<Gtk::ListStore> articles;

   Gtk::Entry& txtName;
   Gtk::Entry& txtArticle;

   Gtk::Button& addName;
   Gtk::Button& deleteName;
   Gtk::Button& addArticle;
   Gtk::Button& deleteArticle;

   Gtk::TreeView& lstNames;
   Gtk::TreeView& lstArticles;
};

#endif
