// PROJECT     : CDManager
// SUBSYSTEM   : Films
// REFERENCES  :
// TODO        : - Used edit-fields instead of labels; to reuse dialog for edit of info
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 19.03.2010
// COPYRIGHT   : Copyright (C) 2010 - 2013, 2026

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

#include <array>
#include <ranges>

#include <glibmm/convert.h>
#include <glibmm/main.h>

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include <YGP/Trace.h>

#include <XGP/XDialog.h>

#include "IMDbProgress.h"

#include "ImportIMDb.h"

namespace {

/**Class defining the columns for the list of results of the IMDb-search
 * \remarks In an anonymous namespace, as FilmList.h defines another FilmColumns
 */
class FilmColumns : public Gtk::TreeModel::ColumnRecord {
  public:
    FilmColumns() : Gtk::TreeModel::ColumnRecord() {
        add(id);
        add(name);
    }

    Gtk::TreeModelColumn<Glib::ustring> id;
    Gtk::TreeModelColumn<Glib::ustring> name;
};

} // namespace

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ImportFromIMDb::ImportFromIMDb()
    : FilmDataEditor(), client(Gtk::make_managed<Gtk::Grid>()), txtID(Gtk::make_managed<Gtk::Entry>()),
      lblDirector(Gtk::make_managed<Gtk::Label>(Glib::ustring())), lblFilm(Gtk::make_managed<Gtk::Label>(Glib::ustring())),
      lblGenre(Gtk::make_managed<Gtk::Label>(Glib::ustring())) {
    set_title(_("Import from IMDb.com"));

    client->set_row_spacing(5);
    client->set_column_spacing(5);
    client->set_margin(5);

    Gtk::Label* lbl(Gtk::make_managed<Gtk::Label>(_("Film (_Name, number or URL):"), true));
    lbl->set_mnemonic_widget(*txtID);
    client->attach(*lbl, 0, 0);
    txtID->set_hexpand(true);
    client->attach(*txtID, 1, 0);

    get_content_area()->append(*client);

    txtID->set_activates_default();
    txtID->signal_changed().connect(sigc::mem_fun(*this, &ImportFromIMDb::inputChanged));
    ok->set_label(_("_Next"));
    inputChanged();

    const std::array values {lblDirector, lblFilm, lblGenre};
    constexpr std::array titles {N_("Director:"), N_("Film:"), N_("Genre:")};
    for (const auto [row, value, title] : std::views::zip(std::views::iota(2), values, titles)) {
        lbl = Gtk::make_managed<Gtk::Label>(_(title));
        lbl->set_halign(Gtk::Align::START);
        client->attach(*lbl, 0, row);
        value->set_halign(Gtk::Align::START);
        client->attach(*value, 1, row);
    }

    show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ImportFromIMDb::~ImportFromIMDb() = default;

//-----------------------------------------------------------------------------
/// Callback, if one of the edit-fields is changed
//-----------------------------------------------------------------------------
void ImportFromIMDb::inputChanged() {
    contract_assert(ok);
    ok->set_sensitive(txtID->get_text_length() != 0);
}

//-----------------------------------------------------------------------------
/// Callback, if one entry of the list box is (de)selected
/// \param list: List to examine
//-----------------------------------------------------------------------------
void ImportFromIMDb::rowSelected(Gtk::TreeView* list) {
    contract_assert(ok);
    if (Gtk::TreeModel::iterator sel(list->get_selection()->get_selected()); sel) {
        TRACE7("ImportFromIMDb::rowSelected (Gtk::TreeView*) - " << bool(sel->parent()))
        ok->set_sensitive(bool(sel->parent()));
    }
    else
        ok->set_sensitive(false);
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a button in the dialog
//-----------------------------------------------------------------------------
void ImportFromIMDb::okEvent() {
    TRACE1("ImportFromIMDb::okEvent () - " << status);

    switch (status) {
    case QUERY: {
        status = LOADING;
        txtID->set_sensitive(false);
        ok->set_sensitive(false);

        auto* progress(Gtk::make_managed<IMDbProgress>(Glib::locale_from_utf8(txtID->get_text())));
        progress->sigError.connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::showError), progress));
        // progress->sigAmbiguous.connect (sigc::bind (sigc::mem_fun (*this, &ImportFromIMDb::showSearchResults), progress));
        progress->sigSuccess.connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::showData), progress));
        progress->set_hexpand(true);
        client->attach(*progress, 0, 1, 2, 1);
        break;
    }

    case CHOOSING:
        // Do nothing; any action is performed by other handlers
        break;

    case CONFIRM:
        // Called while handling the OK-response; so the dialog is freed after returning
        finished = saveIMDbInfo();
        break;

    default:
        TRACE1("Status: " << status);
        contract_assert(false);
    }
}

//-----------------------------------------------------------------------------
/// Informs listeners about the information in the dialog
/// \returns bool True, if all listeners successfully handled the update
//-----------------------------------------------------------------------------
bool ImportFromIMDb::saveIMDbInfo() {
    contract_assert(lblDirector);
    contract_assert(lblFilm);
    contract_assert(lblGenre);
    contract_assert(image);
    std::string icon(getIcon());
    return sigLoaded.emit(lblDirector->get_text(), lblFilm->get_text(), lblGenre->get_text(), getSummary(), icon);
}

//-----------------------------------------------------------------------------
/// Removes the progressbar
/// \param client Client area from which remove the progressbar from
/// \param progress Progressbar to remove
//-----------------------------------------------------------------------------
void ImportFromIMDb::removeProgressBar(Gtk::Grid* client, IMDbProgress* progress) {
    TRACE9("ImportFromIMDb::removeProgressBar (Gtk::Grid*, IMDbProgress*)");
    stopLoading(progress);
    client->remove(*progress); // Frees the (managed) progressbar
}

//-----------------------------------------------------------------------------
/// Stops the loading of data of the progressbar
/// \param progress Progressbar to stop
//-----------------------------------------------------------------------------
void ImportFromIMDb::stopLoading(IMDbProgress* progress) {
    TRACE9("ImportFromIMDb::stopLoading (IMDbProgress*)");
    progress->stop();
}

//-----------------------------------------------------------------------------
/// Adds an icon to the film information
/// \param bufImage Image description
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::addIcon(const std::string& bufImage, IMDbProgress* progress) {
    TRACE1("ImportFromIMDb::addIcon (const std::string&, IMDbProgress*) - " << bufImage.length());

    FilmDataEditor::setIcon(bufImage);

    status = CONFIRM;
    ok->set_sensitive();
    progress->hide();
    Glib::signal_idle().connect_once([client = client, progress] { removeProgressBar(client, progress); });
}

//-----------------------------------------------------------------------------
/// Adds an icon to the film information
/// \param image Image description
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::loadIcon(const std::string& image, IMDbProgress* progress) {
    TRACE1("ImportFromIMDb::loadIcon (const std::string&, IMDbProgress*) - " << image);
    status = IMGLOAD;
    progress->sigIcon.connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::addIcon), progress));
    progress->start(image, true);
}

//-----------------------------------------------------------------------------
/// Updates the dialog with the data read
/// \param entry Found IMDbEntry
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showData(const IMDbProgress::IMDbEntry& entry, IMDbProgress* progress) {
    TRACE9("ImportFromIMDb::showData (3x const Glib::ustring&, IMDbProgress*) - " << entry.title);
    contract_assert(client);

    if (entry.image.size()) {
        Glib::signal_idle().connect_once([progress] { stopLoading(progress); });
        Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::loadIcon), entry.image, progress));
    }
    else {
        status = CONFIRM;
        progress->hide();
        Glib::signal_idle().connect_once([client = client, progress] { removeProgressBar(client, progress); });
    }

    lblDirector->set_text(entry.director);
    lblFilm->set_text(entry.title);
    lblGenre->set_text(entry.genre);
    setSummary(entry.summary);

    // While the poster is loading OK stays disabled; addIcon() or showError() enable it
    ok->set_label(_("_OK"));
    ok->set_sensitive(status == CONFIRM);
}

//-----------------------------------------------------------------------------
/// Displays an error-message and makes the progress-bar stop
/// \param msg Message to display
/// \param progress Progress bar used for displaying the status; will be removed
//-----------------------------------------------------------------------------
void ImportFromIMDb::showError(const Glib::ustring& msg, IMDbProgress* progress) {
    TRACE9("ImportFromIMDb::showError (const Glib::ustring&, IMDbProgress*) - " << msg);
    Gtk::MessageDialog dlg(*this, msg, false, Gtk::MessageType::ERROR);
    XGP::runModal(dlg);
    Glib::signal_idle().connect_once([client = client, progress] { removeProgressBar(client, progress); });

    if (status == IMGLOAD) {
        // Only the poster failed; the film data is still usable, so continue without it
        status = CONFIRM;
        progress->hide();
        ok->set_label(_("_OK"));
        ok->set_sensitive();
    }
    else {
        status = QUERY;
        inputChanged();
        txtID->set_sensitive();
    }
}

//-----------------------------------------------------------------------------
/// Shows the results of an IMDb-search
/// \param results Map containing found entries (ID/name)
/// \param progress Progress bar used for displaying the status; will be hidden
//-----------------------------------------------------------------------------
void ImportFromIMDb::showSearchResults(const IMDbProgress::IMDbMatchData& results, IMDbProgress* progress) {
    contract_assert(client);
    progress->hide();
    Glib::signal_idle().connect_once([progress] { stopLoading(progress); });

    FilmColumns colFilms;
    Glib::RefPtr<Gtk::TreeStore> model(Gtk::TreeStore::create(colFilms));
    Gtk::ScrolledWindow& scrl(*Gtk::make_managed<Gtk::ScrolledWindow>());
    Gtk::TreeView& list(*Gtk::make_managed<Gtk::TreeView>(model));

    // Fill the lines into the list
    const std::array<Glib::ustring, 1> matches {_("Titles")};
    for (const auto [i, matchName] : std::views::enumerate(matches)) {
        TRACE1("showSearchResults " << matchName)
        const IMDbProgress::IMDbSearchEntries& films(results.at(static_cast<IMDbProgress::match>(i)));
        if (!films.empty()) {
            Gtk::TreeModel::Row match(*model->append());
            match[colFilms.name] = matchName;

            for (const auto& film : films) {
                Gtk::TreeModel::Row row(*model->append(match.children()));
                TRACE5("showSearchResults - " << film.url << '/' << film.title);
                row[colFilms.id] = film.url;
                row[colFilms.name] = Glib::ustring(film.title);
            }

            if (i != std::ssize(matches) - 1)
                list.expand_row(model->get_path(match.get_iter()), false);
        }
    }

    scrl.set_has_frame(true);
    scrl.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrl.set_child(list);
    scrl.set_expand(true);
    list.append_column(_("Film"), colFilms.name);
    list.set_size_request(-1, 150);
    list.signal_row_activated().connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::rowActivated), &scrl, &list, progress));

    client->attach(scrl, 0, 2, 2, 5);
    image->hide();

    list.grab_focus();
    list.get_selection()->signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::rowSelected), &list));

    status = CHOOSING;
    connOK =
        ok->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ImportFromIMDb::continueLoading), &scrl, &list, progress));
}

//-----------------------------------------------------------------------------
/// Continues loading with the selected list-entry
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::continueLoading(Gtk::ScrolledWindow* scrl, Gtk::TreeView* list, IMDbProgress* progress) {
    Gtk::TreeModel::iterator sel(list->get_selection()->get_selected());
    if (!sel)
        return;

    Gtk::TreeRow row(*sel);
    loadRow(row, scrl, list, progress);
}

//-----------------------------------------------------------------------------
/// Continues with loading the film identified by the passed row
/// \param row Row to load
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::loadRow(Gtk::TreeRow& row, Gtk::ScrolledWindow* scrl, [[maybe_unused]] Gtk::TreeView* list,
                             IMDbProgress* progress) {
    contract_assert(client);

    contract_assert(connOK.connected());
    connOK.disconnect();
    scrl->hide();
    client->remove(*scrl);

    progress->show();
    progress->start(row.get_value(FilmColumns().id));
}

//-----------------------------------------------------------------------------
/// Callback after double-clicking a row; continues loading this entry
/// \param path Activated path
/// \param column Column in path
/// \param scrl Scrolledwindow containing the list
/// \param list List to get entry to load from
/// \param progress Progressbar to load
//-----------------------------------------------------------------------------
void ImportFromIMDb::rowActivated(const Gtk::TreePath& path, Gtk::TreeViewColumn*, Gtk::ScrolledWindow* scrl,
                                  Gtk::TreeView* list, IMDbProgress* progress) {
    Gtk::TreeRow row(*(list->get_model()->get_iter(path)));
    loadRow(row, scrl, list, progress);
}

//-----------------------------------------------------------------------------
/// Searches for the passed film
/// \param film Information of the film to search for
//-----------------------------------------------------------------------------
void ImportFromIMDb::searchFor(const Glib::ustring& film) {
    txtID->set_text(film);

    Glib::ustring empty;
    lblDirector->set_text(empty);
    lblFilm->set_text(empty);
    lblGenre->set_text(empty);
    setSummary(empty);
    image->clear();

    status = QUERY;
    ok->set_label(_("_Next"));
    okEvent();
}
