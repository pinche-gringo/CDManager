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

#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/treepath.h>

#include "FilmData.h"
#include "IMDbProgress.h"

namespace Gtk {
class Label;
class TreeRow;
class TreeView;
class TreeViewColumn;
class ScrolledWindow;
} // namespace Gtk

/**Dialog allowing to import information from a film from IMDb.com
 *
 * After entering an film identification (a number or the URL to
 * IMDb.com) the matching page on IMDb.com is read and the relevant
 * information is filtered out and displayed for confirmation.
 */
class ImportFromIMDb : public FilmDataEditor {
  public:
    ImportFromIMDb();
    ~ImportFromIMDb() override;

    /// Creates the dialog
    /// \remarks Cares also about freeing the dialog
    static ImportFromIMDb* create() {
        ImportFromIMDb* dlg(new ImportFromIMDb);
        dlg->signal_response().connect(sigc::mem_fun(*dlg, &ImportFromIMDb::free));
        return dlg;
    }

    void searchFor(const Glib::ustring& info);

    /// Signal emitted, when the loaded film-information is confirmed
    sigc::signal<bool(const Glib::ustring&, const Glib::ustring&, const Glib::ustring&, const Glib::ustring&, std::string&)>
        sigLoaded;

    // Prohibited manager functions
    ImportFromIMDb(const ImportFromIMDb&) = delete;
    const ImportFromIMDb& operator=(const ImportFromIMDb&) = delete;

  protected:
    Gtk::Grid* client;       ///< Pointer to the client information area
    Gtk::Entry* txtID;       ///< Textfield, where user enters the ID
    Gtk::Label* lblDirector; ///< Label displaying the director
    Gtk::Label* lblFilm;     ///< Label displaying the film (with year)
    Gtk::Label* lblGenre;    ///< Label displaying the genre of the film

    void okEvent() override;

  private:
    volatile enum { QUERY, LOADING, CHOOSING, CONFIRM, IMGLOAD } status {QUERY};

    static void removeProgressBar(Gtk::Grid* client, IMDbProgress* progress) pre(client != nullptr) pre(progress != nullptr);
    static void stopLoading(IMDbProgress* progress) pre(progress != nullptr);
    void continueLoading(Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);
    void loadSelection(Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress);
    void rowActivated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column, Gtk::ScrolledWindow* scrl, Gtk::TreeView* list,
                      IMDbProgress* progress) pre(list != nullptr);
    void rowSelected(Gtk::TreeView* list) pre(list != nullptr);
    void loadRow(Gtk::TreeRow& row, Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress)
        pre(scrl != nullptr) pre(list != nullptr) pre(progress != nullptr);

    void inputChanged();
    void showError(const Glib::ustring& msg, IMDbProgress* progress);
    void showSearchResults(const IMDbProgress::IMDbMatchData& results, IMDbProgress* progress) pre(progress != nullptr);
    void showData(const IMDbProgress::IMDbEntry& entry, IMDbProgress* progress) pre(progress != nullptr);
    void addIcon(const std::string& bufImage, IMDbProgress* progress);
    void loadIcon(const std::string& image, IMDbProgress* progress) pre(progress != nullptr) pre(!image.empty());
    bool saveIMDbInfo();

    sigc::connection connOK;
};

#endif
