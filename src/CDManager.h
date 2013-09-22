#ifndef CDMANAGER_H
#define CDMANAGER_H

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

#if !defined (WITH_ACTORS) || !defined (WITH_RECORDS) || !defined (WITH_FILMS)
#  error Need WITH_ACTORS, WITH_RECORDS and WITH_FILMS defined
#endif

#include <map>
#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/statusbar.h>

#include "Genres.h"

#include <YGP/Relation.h>

#include <XGP/XApplication.h>


// Forward declarations
class NBPage;
class Options;
namespace YGP {
   class Entity;
   class StatusObject;
}


/**Class for application to manage CDs (audio and video)
*/
class CDManager : public XGP::XApplication {
 public:
   // Manager functions
   CDManager (Options& options);
   ~CDManager ();

 private:
   // Protected manager functions
   CDManager (const CDManager&);
   const CDManager& operator= (const CDManager&);

   // Event-handling
   void save ();
   void showLogin ();
   void logout ();
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
   void export2HTML ();
#endif

   void showStatistics ();
   void editPreferences ();
   void savePreferences ();

   virtual void showAboutbox ();
   virtual const char* getHelpfile ();
   void pageSwitched (Gtk::Widget* page, guint iPage);

   bool login (const Glib::ustring& user, const Glib::ustring& pwd);
   void loadDatabase ();

   void enableMenus (bool enable);
   void exit ();
   bool on_delete_event (GdkEventAny*);

   static const char* xpmProgram[];
   static const char* xpmAuthor[];

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;

   static const char* const DBNAME;

   Genres recGenres;
   Genres filmGenres;

   Gtk::Notebook  nb;
   Gtk::Statusbar status;

   enum { LOGIN = 0, SAVE, LOGOUT, MEDIT, STATISTICS, SAVE_PREFS,
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
	  EXPORT,
#endif
   	  LAST };
   Glib::RefPtr<Gtk::Action> apMenus[LAST];
   Options& opt;

   NBPage* pages[WITH_ACTORS + WITH_FILMS + WITH_RECORDS];
};

#endif
