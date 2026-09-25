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

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include <YGP/Check.h>
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
    Check3(actor);

    for (std::vector<HFilm>::const_iterator i(films.begin()); i != films.end(); ++i)
        insertFilm(*i);

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
RelateFilm::~RelateFilm() {}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void RelateFilm::okEvent() {
    Check3(actor);

    std::vector<HFilm> films;
    for (Gtk::TreeModel::const_iterator i(mFilms->children().begin()); i != mFilms->children().end(); ++i)
        films.push_back(i->get_value(colFilms.hFilm));
    signalRelateFilms.emit(actor, films);
}

//-----------------------------------------------------------------------------
/// Adds a film to the ones starring the actor
/// \param path: Path to film to add
//-----------------------------------------------------------------------------
void RelateFilm::addFilm(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
    TRACE7("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");
    Check3(lstAllFilms.get_model());

    Gtk::TreeModel::iterator sel(availFilms->get_iter(path));
    Check3(sel);
    if (sel->parent()) {
        HEntity entry(sel->get_value(colAllFilms.entry));
        Check3(entry);
        HFilm film(boost::dynamic_pointer_cast<Film>(entry));
        Check3(film);
        TRACE9("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << film->getName());
        insertFilm(film);
    }
    else {
        for (Gtk::TreeModel::iterator i(sel->children().begin()); i != sel->children().end(); ++i) {
            HEntity entry(i->get_value(colAllFilms.entry));
            Check3(entry);
            HFilm film(boost::dynamic_pointer_cast<Film>(entry));
            Check3(film);
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
    Check1(film);

    // Check that film does not exist
    for (Gtk::TreeModel::const_iterator i(mFilms->children().begin()); i != mFilms->children().end(); ++i)
        if (film == i->get_value(colFilms.hFilm))
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

    Glib::RefPtr<Gtk::TreeSelection> filmSel(lstAllFilms.get_selection());
    std::vector<Gtk::TreePath> list(filmSel->get_selected_rows());
    for (std::vector<Gtk::TreePath>::iterator i(list.begin()); i != list.end(); ++i)
        addFilm(*i, nullptr);
}

//-----------------------------------------------------------------------------
/// Remove all selected films
//-----------------------------------------------------------------------------
void RelateFilm::removeSelected() {
    TRACE9("RelateFilm::removeSelected ()");

    Glib::RefPtr<Gtk::TreeSelection> filmSel(lstFilms.get_selection());
    std::vector<Gtk::TreePath> list(filmSel->get_selected_rows());
    for (std::vector<Gtk::TreePath>::iterator i(list.begin()); i != list.end(); ++i)
        removeFilm(*i, nullptr);
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
    Check2(actor);
    Glib::ustring title(_("Films starring %1"));
    title.replace(title.find("%1"), 2, actor->getName());
    set_title(title);
    set_default_size(580, 450);

    Check1(mFilms);
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
