#ifndef CDMANAGER_H
#define CDMANAGER_H

//$Id: CDManager.h,v 1.2 2004/10/17 03:10:13 markus Exp $

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
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/scrolledwindow.h>

#include <XGP/XApplication.h>


/**Class holding the columns in the record listbox
 */
class RecordColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   RecordColumns () {
      add (name); add (band); add (year); add (genre);
   }

   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> band;
   Gtk::TreeModelColumn<unsigned int>  year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class holding the columns in the song listbox
 */
class SongColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   SongColumns () {
      add (name); add (duration);
   }

   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> duration;
};


/**Class for application to manager CDs (audio and video)
*/
class CDManager : public XGP::XApplication {
 public:
   // Manager functions
   CDManager ();
   ~CDManager ();

 private:
   // IDs for menus
   enum { LOGIN = XApplication::LAST, LOGOUT, MEDIT, NEW, EDIT, DELETE, EXIT };

   // Protected manager functions
   CDManager (const CDManager&);
   const CDManager& operator= (const CDManager&);

   // Event-handling
   virtual void command (int menu);
   virtual void showAboutbox ();
   virtual const char* getHelpfile ();

   bool login (const Glib::ustring& user, const Glib::ustring& pwd);
   void loadDatabase ();
   void enableMenus (bool enable);
   void enableEdit (bool enable);

   static XGP::XApplication::MenuEntry menuItems[];

   static const char* xpmProgram[];
   static const char* xpmAuthor[];

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;

   static const char* const DBNAME;

   Gtk::Notebook  nb;
   Gtk::Table     cds;

   SongColumns                  colSongs;
   RecordColumns                colRecords;
   Glib::RefPtr<Gtk::ListStore> mSongs;
   Glib::RefPtr<Gtk::ListStore> mRecords;
   Gtk::TreeView                records;
   Gtk::TreeView                songs;
   Gtk::ScrolledWindow          scrlSongs;
   Gtk::ScrolledWindow          scrlRecords;

   Gtk::Table     movies;
   Gtk::Statusbar status;
};

#endif
