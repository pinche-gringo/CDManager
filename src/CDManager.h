#ifndef CDMANAGER_H
#define CDMANAGER_H

//$Id: CDManager.h,v 1.16 2004/11/17 17:37:42 markus Exp $

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
#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/paned.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/scrolledwindow.h>

#include "Song.h"
#include "Record.h"
#include "SongList.h"
#include "Interpret.h"
#include "RecordList.h"

#include <YGP/Relation.h>

#include <XGP/XApplication.h>


// Forward declarations
namespace YGP {
   class Entity;
}


/**Class for application to manager CDs (audio and video)
*/
class CDManager : public XGP::XApplication {
 public:
   // Manager functions
   CDManager ();
   ~CDManager ();

 private:
   // IDs for menus
   enum { LOGIN = XApplication::LAST, SAVE, LOGOUT, MEDIT, NEW_ARTIST,
	  NEW_RECORD, NEW_SONG, DELETE, EXIT };

   // Protected manager functions
   CDManager (const CDManager&);
   const CDManager& operator= (const CDManager&);

   // Event-handling
   virtual void command (int menu);
   virtual void showAboutbox ();
   virtual const char* getHelpfile ();
   void recordSelected ();

   typedef enum { NONE_SELECTED, ARTIST_SELECTED, RECORD_SELECTED } SELECTED;
   void enableEdit (SELECTED selected);
   bool login (const Glib::ustring& user, const Glib::ustring& pwd);
   void loadDatabase ();
   void enableMenus (bool enable);
   void loadSongs (const HRecord& record);

   void artistChanged (const HInterpret& artist);
   void recordChanged (const HRecord& record);
   void recordGenreChanged (const HRecord& record);
   void songChanged (const HSong& song);
   HInterpret getInterpret (unsigned int nr) const;

   void deleteRecord (const Gtk::TreeIter& record);
   void deleteSelectedRecords ();
   void deleteSelectedSongs ();

   void removeDeletedEntries ();
   void writeChangedEntries ();

   static XGP::XApplication::MenuEntry menuItems[];

   static const char* xpmProgram[];
   static const char* xpmAuthor[];

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;

   static const char* const DBNAME;

   std::vector<HInterpret>               artists;
   std::map<unsigned int, Glib::ustring> genres;

   YGP::Relation1_N<HInterpret, HRecord> relRecords;
   YGP::Relation1_N<HRecord, HSong>      relSongs;

   Gtk::Notebook  nb;
   Gtk::HPaned    cds;
   Gtk::Table     movies;

   RecordList                   records;
   SongList                     songs;
   Gtk::ScrolledWindow          scrlSongs;
   Gtk::ScrolledWindow          scrlRecords;

   Gtk::Statusbar status;

   std::vector<HSong>      deletedSongs;
   std::vector<HRecord>    deletedRecords;
   std::vector<HInterpret> deletedInterprets;

   std::map<HSong,HSong>            changedSongs;
   std::map<HRecord, HRecord>       changedRecords;
   std::map<HInterpret, HInterpret> changedInterprets;
};

#endif
