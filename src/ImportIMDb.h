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


#include <gtkmm/grid.h>
#include <gtkmm/entry.h>
#include <gtkmm/treepath.h>

#include "IMDbProgress.h"
#include "FilmData.h"

namespace Gtk {
   class Label;
   class TreeRow;
   class TreeView;
   class TreeViewColumn;
   class ScrolledWindow;
}


/**Dialog allowing to import information from a film from IMDb.com
 *
 * After entering an film identification (a number or the URL to
 * IMDb.com) the matching page on IMDb.com is read and the relevant
 * information is filtered out and displayed for confirmation.
 */
class ImportFromIMDb : public FilmDataEditor {
 public:
   ImportFromIMDb();
   virtual ~ImportFromIMDb();

   /// Creates the dialog
   /// \remarks Cares also about freeing the dialog
   static ImportFromIMDb* create() {
      ImportFromIMDb* dlg(new ImportFromIMDb);
      dlg->signal_response().connect(sigc::mem_fun(*dlg, &ImportFromIMDb::free));
      return dlg;
   }

   void searchFor(const Glib::ustring& info);

   /// Signal emitted, when the loaded film-information is confirmed
   sigc::signal<bool (const Glib::ustring&, const Glib::ustring&,
		      const Glib::ustring&, const Glib::ustring&, std::string&)> sigLoaded;

   // Prohibited manager functions
   ImportFromIMDb(const ImportFromIMDb&) = delete;
   const ImportFromIMDb& operator=(const ImportFromIMDb&) = delete;

 protected:
   Gtk::Grid* client;             ///< Pointer to the client information area
   Gtk::Entry* txtID;                ///< Textfield, where user enters the ID
   Gtk::Label* lblDirector;                ///< Label displaying the director
   Gtk::Label* lblFilm;            ///< Label displaying the film (with year)
   Gtk::Label* lblGenre;          ///< Label displaying the genre of the film

   void okEvent() override;

 private:
   volatile enum { QUERY, LOADING, CHOOSING, CONFIRM, IMGLOAD } status;

   static void removeProgressBar(Gtk::Grid* client, IMDbProgress* progress);
   static void stopLoading(IMDbProgress* progress);
   void continueLoading(Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);
   void loadSelection(Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);
   void rowActivated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column, Gtk::ScrolledWindow* scrl,
		     Gtk::TreeView* list, IMDbProgress* progress);
   void rowSelected(Gtk::TreeView* list);
   void loadRow(Gtk::TreeRow& row, Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);

   void inputChanged();
   void showError(const Glib::ustring& msg, IMDbProgress* progress);
   void showSearchResults(const IMDbProgress::IMDbMatchData& results, IMDbProgress* progress);
   void showData(const IMDbProgress::IMDbEntry& entry, IMDbProgress* progress);
   void addIcon(const std::string& bufImage, IMDbProgress* progress);
   void loadIcon(const std::string& image, IMDbProgress* progress);
   bool saveIMDbInfo();

   sigc::connection connOK;
};

#endif
