#ifndef CDMANAGER_H
#define CDMANAGER_H

//$Id: CDManager.h,v 1.44 2006/03/05 22:37:15 markus Rel $

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


#include <cdmgr-cfg.h>

#if !defined (WITH_ACTORS) || !defined (WITH_RECORDS) || !defined (WITH_MOVIES)
#  error Need WITH_ACTORS, WITH_RECORDS and WITH_MOVIES defined
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
#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
   void export2HTML ();
#endif

   void editPreferences ();
   void savePreferences ();

   virtual void showAboutbox ();
   virtual const char* getHelpfile ();
   void pageSwitched (GtkNotebookPage* page, guint iPage);

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
   Genres movieGenres;

   Gtk::Notebook  nb;
   Gtk::Statusbar status;

   enum { LOGIN = 0, SAVE, LOGOUT, MEDIT, SAVE_PREFS,
#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
	  EXPORT,
#endif
   	  LAST };
   Glib::RefPtr<Gtk::Action> apMenus[LAST];
   Options& opt;

   NBPage* pages[WITH_ACTORS + WITH_MOVIES + WITH_RECORDS];
};

#endif
