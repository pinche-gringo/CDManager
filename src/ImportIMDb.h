#ifndef IMPORTIMDB_H
#define IMPORTIMDB_H

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


#include <gtkmm/table.h>

#include <YGP/Check.h>
#include <XGP/XDialog.h>

#include "IMDbProgress.h"

namespace Gtk {
   class Label;
   class Table;
   class TreeView;
}


/**Dialog allowing to import information from a movie from IMDb.com
 *
 * After entering an movie identification (a number or the URL to
 * IMDb.com) the matching page on IMDb.com is read and the relevant
 * information is filtered out and displayed for confirmation.
 */
class ImportFromIMDb : public XGP::XDialog {
 public:
   ImportFromIMDb ();
   virtual ~ImportFromIMDb ();

   /// Creates the dialog (and set it as child of the parent)
   /// \remarks Cares also about freeing the dialog
   static ImportFromIMDb* create () {
      ImportFromIMDb* dlg (new ImportFromIMDb);
      dlg->signal_response ().connect (mem_fun (*dlg, &ImportFromIMDb::free));
      return dlg;
   }

   /// Signal emitted, when the loaed movie-information is confirmed
   sigc::signal<bool, const Glib::ustring&, const Glib::ustring&, const Glib::ustring&> sigLoaded;

 protected:
   Gtk::Table* client;            ///< Pointer to the client information area
   Gtk::Entry* txtID;                ///< Textfield, where user enters the ID
   Gtk::Label* lblDirector;                ///< Label displaying the director
   Gtk::Label* lblMovie;          ///< Label displaying the movie (with year)
   Gtk::Label* lblGenre;         ///< Label displaying the genre of the movie
   Gtk::Label* lblSummary;      ///< Label displaying the summary of the plot

   Glib::ustring contentIMDb;                ///< Page received from IMDb.com

   void okEvent ();

 private:
   volatile enum { QUERY, LOADING, CHOOSING, CONFIRM } status;

   // Prohibited manager functions
   ImportFromIMDb (const ImportFromIMDb& other);
   const ImportFromIMDb& operator= (const ImportFromIMDb& other);

   static bool removeProgressBar (Gtk::Table* client, IMDbProgress* progress);
   static bool stopLoading (IMDbProgress* progress);
   void continueLoading (Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);

   void rowSelected (Gtk::TreeView* list);

   void inputChanged ();
   void showError (const Glib::ustring& msg, IMDbProgress* progress);
   void showSearchResults (const std::map<IMDbProgress::match, IMDbProgress::IMDbSearchEntries>& results,
			   IMDbProgress* progress);
   void showData (const IMDbProgress::IMDbEntry& entry, IMDbProgress* progress);

   sigc::connection connOK;
};

#endif
