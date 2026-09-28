// PROJECT     : CDManager
// SUBSYSTEM   : Actor
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 2005-10-18
// COPYRIGHT   : Copyright (C) 2005, 2009 - 2011, 2026

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

#include <ranges>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include <YGP/Trace.h>

#include "RelateFilm.h"

//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose films should be displayed/changed
/// \param films: Films already connected with the actor
/// \param allFilms: All available films
//-----------------------------------------------------------------------------
RelateFilm::RelateFilm(const HActor& actor, const std::vector<HFilm>& films, const Glib::RefPtr<Gtk::TreeStore> allFilms)
    : XGP::XDialog(OKCANCEL), mFilms(Gtk::ListStore::create(colFilms)), availFilms(allFilms),
      addFilms(*Gtk::make_managed<Gtk::Button>()), removeFilms(*Gtk::make_managed<Gtk::Button>()),
      lstFilms(*Gtk::make_managed<Gtk::TreeView>()), lstAllFilms(*Gtk::make_managed<Gtk::TreeView>()), actor(actor) {
    TRACE9("RelateFilm::RelateFilm (const HActor&, const std::vector<HFilm>&, const Glib::RefPtr<Gtk::TreeStore>)");

    for (const auto& film : films)
        insertFilm(film);

    init();
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose films should be displayed/changed
/// \param films: Films already connected with the actor
/// \param allFilms: All available films
//-----------------------------------------------------------------------------
RelateFilm::RelateFilm(const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allFilms)
    : XGP::XDialog(OKCANCEL), mFilms(Gtk::ListStore::create(colFilms)), availFilms(allFilms),
      addFilms(*Gtk::make_managed<Gtk::Button>()), removeFilms(*Gtk::make_managed<Gtk::Button>()),
      lstFilms(*Gtk::make_managed<Gtk::TreeView>()), lstAllFilms(*Gtk::make_managed<Gtk::TreeView>()), actor(actor) {
    TRACE9("RelateFilm::RelateFilm (const HActor&, const Glib::RefPtr<Gtk::TreeStore>)");
    init();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RelateFilm::~RelateFilm() = default;

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void RelateFilm::okEvent() {
    contract_assert(actor);

    std::vector<HFilm> films;
    for (const auto& row : mFilms->children())
        films.push_back(row.get_value(colFilms.hFilm));
    signalRelateFilms.emit(actor, films);
}

//-----------------------------------------------------------------------------
/// Adds a film to the ones starring the actor
/// \param path: Path to film to add
//-----------------------------------------------------------------------------
void RelateFilm::addFilm(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
    TRACE7("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");
    contract_assert(lstAllFilms.get_model());

    Gtk::TreeModel::iterator sel(availFilms->get_iter(path));
    contract_assert(sel);
    if (sel->parent()) {
        HEntity entry(sel->get_value(colAllFilms.entry));
        contract_assert(entry);
        HFilm film(std::dynamic_pointer_cast<Film>(entry));
        contract_assert(film);
        TRACE9("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << film->getName());
        insertFilm(film);
    }
    else {
        for (const auto& child : sel->children()) {
            HEntity entry(child.get_value(colAllFilms.entry));
            contract_assert(entry);
            HFilm film(std::dynamic_pointer_cast<Film>(entry));
            contract_assert(film);
            TRACE9("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << film->getName())
            insertFilm(film);
        }
    }
}

//-----------------------------------------------------------------------------
/// Adds a film to the ones starring the actor
/// \param path: Path to film to add
//-----------------------------------------------------------------------------
void RelateFilm::removeFilm(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
    TRACE9("RelateFilm::removeFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");

    mFilms->erase(mFilms->get_iter(path));
}

//-----------------------------------------------------------------------------
/// Adds the passed film/director to the film-list for the actor
/// \param film: Film to add
/// \param director: Director of the film
//-----------------------------------------------------------------------------
void RelateFilm::insertFilm(const HFilm& film) {
    TRACE9("RelateFilm::insertFilm (const HFilm&) - " << (film ? film->getName().c_str() : ""));

    // Check that film does not exist
    for (const auto& row : mFilms->children())
        if (film == row.get_value(colFilms.hFilm))
            return;

    Gtk::TreeModel::Row newFilm(*mFilms->append());
    newFilm[colFilms.hFilm] = film;
    newFilm[colFilms.film] = film->getName();
}

//-----------------------------------------------------------------------------
/// Adds all selected films
//-----------------------------------------------------------------------------
void RelateFilm::addSelected() {
    TRACE9("RelateFilm::addSelected ()");

    for (const auto& path : lstAllFilms.get_selection()->get_selected_rows())
        addFilm(path, nullptr);
}

//-----------------------------------------------------------------------------
/// Remove all selected films
//-----------------------------------------------------------------------------
void RelateFilm::removeSelected() {
    TRACE9("RelateFilm::removeSelected ()");

    // Erase from the end, so the paths of the remaining selected rows stay valid
    for (const auto& path : lstFilms.get_selection()->get_selected_rows() | std::views::reverse)
        removeFilm(path, nullptr);
}

//-----------------------------------------------------------------------------
/// Callback after changing the films-selection. Used to enable/disable the
/// remove-button
//-----------------------------------------------------------------------------
void RelateFilm::filmsSelected() { removeFilms.set_sensitive(!lstFilms.get_selection()->get_selected_rows().empty()); }

//-----------------------------------------------------------------------------
/// Callback after changing the allFilms-selection. Used to enable/disable the
/// add-button
//-----------------------------------------------------------------------------
void RelateFilm::allFilmsSelected() { addFilms.set_sensitive(!lstAllFilms.get_selection()->get_selected_rows().empty()); }

//-----------------------------------------------------------------------------
/// Inititalizes the class
//-----------------------------------------------------------------------------
void RelateFilm::init() {
    contract_assert(actor);
    set_title(Glib::ustring::compose(_("Films starring %1"), actor->getName()));
    set_default_size(580, 450);

    contract_assert(mFilms);
    lstFilms.set_model(mFilms);
    lstAllFilms.set_model(availFilms);

    Gtk::ScrolledWindow& scrlFilms(*Gtk::make_managed<Gtk::ScrolledWindow>());
    Gtk::ScrolledWindow& scrlAllFilms(*Gtk::make_managed<Gtk::ScrolledWindow>());
    scrlFilms.set_has_frame(true);
    scrlAllFilms.set_has_frame(true);
    scrlFilms.set_child(lstFilms);
    scrlAllFilms.set_child(lstAllFilms);
    scrlFilms.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrlAllFilms.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrlFilms.set_expand(true);
    scrlAllFilms.set_expand(true);

    lstFilms.append_column(_("Film"), colFilms.film);
    lstAllFilms.append_column(_("Available directors/films"), colAllFilms.name);

    Glib::RefPtr<Gtk::TreeSelection> sel(lstFilms.get_selection());
    sel->set_mode(Gtk::SelectionMode::MULTIPLE);
    sel->signal_changed().connect(sigc::mem_fun(*this, &RelateFilm::filmsSelected));
    filmsSelected();

    sel = lstAllFilms.get_selection();
    sel->set_mode(Gtk::SelectionMode::MULTIPLE);
    sel->signal_changed().connect(sigc::mem_fun(*this, &RelateFilm::allFilmsSelected));
    allFilmsSelected();

    lstFilms.signal_row_activated().connect(sigc::mem_fun(*this, &RelateFilm::removeFilm));
    lstAllFilms.signal_row_activated().connect(sigc::mem_fun(*this, &RelateFilm::addFilm));
    lstAllFilms.expand_all();

    Gtk::Box& bbox(*Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5));
    bbox.set_valign(Gtk::Align::CENTER);
    addFilms.set_icon_name("go-previous");
    removeFilms.set_icon_name("go-next");

    bbox.append(addFilms);
    bbox.append(removeFilms);
    addFilms.signal_clicked().connect(sigc::mem_fun(*this, &RelateFilm::addSelected));
    removeFilms.signal_clicked().connect(sigc::mem_fun(*this, &RelateFilm::removeSelected));

    Gtk::Box& box(*Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5));
    box.append(scrlFilms);
    box.append(bbox);
    box.append(scrlAllFilms);
    box.set_margin(5);
    box.set_expand(true);
    get_content_area()->append(box);

    show();
}
