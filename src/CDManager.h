#ifndef CDMANAGER_H
#define CDMANAGER_H

//$Id: CDManager.h,v 1.7 2004/10/30 14:47:03 markus Rel $

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
#include <gtkmm/treestore.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/scrolledwindow.h>

#include "Song.h"

#include <YGP/Relation.h>

#include <XGP/XApplication.h>


// Forward declarations
namespace YGP {
   class Entity;
}



/**Class holding the columns in the record listbox
 */
class RecordColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   RecordColumns () {
      add (entry); add (name); add (year); add (genre);
   }

   Gtk::TreeModelColumn<YGP::IHandle*> entry;
   Gtk::TreeModelColumn<Glib::ustring> name;
   Gtk::TreeModelColumn<Glib::ustring> year;
   Gtk::TreeModelColumn<Glib::ustring> genre;
};


/**Class holding the columns in the song listbox
 */
class SongColumns : public Gtk::TreeModel::ColumnRecord {
 public:
   SongColumns () {
      add (entry); add (name); add (duration);
   }

   Gtk::TreeModelColumn<HSong>         entry;
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
   void recordSelected ();
   void editRecord (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*);
   void recordChanged (HRecord& hRecord);

   bool login (const Glib::ustring& user, const Glib::ustring& pwd);
   void loadDatabase ();
   void enableMenus (bool enable);
   void enableEdit (bool enable);
   void addSong (const HSong& song);
   void loadSongs (const HRecord& record);

   HRecord& getRecordAtPos (const Gtk::TreeIter& i) const;

   bool canSelect (const Glib::RefPtr<Gtk::TreeModel>& model,
		   const Gtk::TreeModel::Path& path, bool);

   static XGP::XApplication::MenuEntry menuItems[];

   static const char* xpmProgram[];
   static const char* xpmAuthor[];

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;

   static const char* const DBNAME;

   YGP::Relation1_N<HInterpret, HRecord> relRecords;
   YGP::Relation1_N<HRecord, HSong>      relSongs;

   Gtk::Notebook  nb;
   Gtk::Table     cds;

   SongColumns                  colSongs;
   RecordColumns                colRecords;
   Glib::RefPtr<Gtk::ListStore> mSongs;
   Glib::RefPtr<Gtk::TreeStore> mRecords;
   Gtk::TreeView                records;
   Gtk::TreeView                songs;
   Gtk::ScrolledWindow          scrlSongs;
   Gtk::ScrolledWindow          scrlRecords;

   Gtk::Table     movies;
   Gtk::Statusbar status;
};

#endif
