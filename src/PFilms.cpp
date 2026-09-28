// PROJECT     : CDManager
// SUBSYSTEM   : Films
// AUTHOR      : Markus Schwab
// CREATED     : 22.01.2006
// COPYRIGHT   : Copyright (C) 2006 - 2019, 2026

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

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>

#include <unistd.h>

#include <glibmm/bytes.h>
#include <glibmm/main.h>

#include <gdkmm/texture.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/popovermenu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XDialog.h>

#include "FilmData.h"
#include "ImportIMDb.h"
#include "LangImg.h"
#include "SaveCeleb.h"
#include "StorageFilm.h"

#include "PFilms.h"

//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling films
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
//-----------------------------------------------------------------------------
PFilms::PFilms(Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave, const Genres& genres)
    : NBPage(status, menuSave), films(genres), relFilms("films") {
    TRACE9("PFilms::PFilms(Gtk::Statusbar&, Glib::RefPtr<Gio::SimpleAction>, const Genres&)");

    auto* scrlFilms(new Gtk::ScrolledWindow);
    scrlFilms->set_has_frame(true);
    scrlFilms->set_child(films);
    scrlFilms->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

    films.signalOwnerChanged.connect(sigc::mem_fun(*this, &PFilms::directorChanged));
    films.signalObjectChanged.connect(sigc::mem_fun(*this, &PFilms::filmChanged));

    contract_assert(films.get_selection());
    films.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &PFilms::filmSelected));

    films.set_has_tooltip();
    films.signal_query_tooltip().connect(sigc::mem_fun(*this, &PFilms::onQueryTooltip), true);

    widget = scrlFilms;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PFilms::~PFilms() { removeMenu(); }

//-----------------------------------------------------------------------------
/// Adds a new direcotor to the list
//-----------------------------------------------------------------------------
void PFilms::newDirector() {
    TRACE5("void PFilms::newDirector()");
    auto director(std::make_shared<Director>());
    Gtk::TreeModel::iterator i(addDirector(director));

    Gtk::TreePath path(films.getModel()->get_path(i));
    films.set_cursor(path, *films.get_column(0), true);
}

//-----------------------------------------------------------------------------
/// Adds the passed director to the list
/// \param hDirector Director to add
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PFilms::addDirector(HDirector& hDirector) {
    TRACE5("void PFilms::addDirector() - " << hDirector->getName());

    directors.push_back(hDirector);
    Gtk::TreeModel::iterator i(films.append(hDirector).get_iter());
    Gtk::TreePath path(films.getModel()->get_path(i));
    films.selectRow(i);

    aUndo.push(Undo(Undo::INSERT, DIRECTOR, 0, hDirector, path, ""));
    apMenus[UNDO]->set_enabled();
    enableSave();
    return i;
}

//-----------------------------------------------------------------------------
/// Adds a new film to the first selected director
//-----------------------------------------------------------------------------
void PFilms::newFilm() {
    TRACE5("void PFilms::newFilm ()");

    Glib::RefPtr<Gtk::TreeSelection> filmSel(films.get_selection());
    contract_assert(filmSel);
    Gtk::TreeModel::iterator p(filmSel->get_selected());
    contract_assert(p);
    if (p->parent())
        p = p->parent();
    TRACE9("void PFilms::newFilm () - Found director " << films.getDirectorAt(p)->getName());

    auto film(std::make_shared<Film>());
    Gtk::TreeModel::iterator i(addFilm(film, p));
    Gtk::TreePath path(films.getModel()->get_path(i));
    films.set_cursor(path, *films.get_column(0), true);
}

//-----------------------------------------------------------------------------
/// Adds a new film to the first selected director
/// \param hFilm Film to add
/// \param pos Position in list where to add the film
/// \returns Gtk::TreeModel::iterator Appended line in the list
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PFilms::addFilm(HFilm& hFilm, const Gtk::TreeModel::iterator& pos) {
    TRACE9("PFilms::addFilm (HFilm&, const Gtk::TreeModel::iterator&) - " << hFilm->getName());

    Gtk::TreeModel::iterator i(films.append(hFilm, *pos).get_iter());
    Gtk::TreePath path(films.getModel()->get_path(i));
    films.expand_row(films.getModel()->get_path(pos), false);
    films.selectRow(i);

    HDirector director(films.getDirectorAt(pos));
    relFilms.relate(director, hFilm);

    aUndo.push(Undo(Undo::INSERT, FILM, 0, hFilm, path, ""));
    apMenus[UNDO]->set_enabled();
    enableSave();
    return i;
}

//-----------------------------------------------------------------------------
/// Callback after selecting a film
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PFilms::filmSelected() {
    TRACE9("PFilms::filmSelected ()");
    contract_assert(films.get_selection());

    if (Gtk::TreeModel::iterator s(films.get_selection()->get_selected()); s) {
        enableEdit(OWNER_SELECTED);
        menuEdit->set_enabled(bool(s->parent()));
    }
    else {
        enableEdit(NONE_SELECTED);
        menuEdit->set_enabled(false);
    }
}

//----------------------------------------------------------------------------
/// Callback when changing a director
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PFilms::directorChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE9("PFilms::directorChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&)\n\t- " << column << '/'
                                                                                                           << oldValue);

    Gtk::TreePath path(films.getModel()->get_path(row));
    aUndo.push(Undo(Undo::CHANGED, DIRECTOR, column, films.getCelebrityAt(row), path, oldValue));

    enableSave();
    apMenus[UNDO]->set_enabled();
}

//----------------------------------------------------------------------------
/// Callback when changing a film
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PFilms::filmChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE9("PFilms::filmChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&)\n\t- " << column << '/'
                                                                                                       << oldValue);

    Gtk::TreePath path(films.getModel()->get_path(row));
    aUndo.push(Undo(Undo::CHANGED, FILM, column, films.getObjectAt(row), path, oldValue));

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Adds the menu-entries for the language-menu to the passed menu. The entries
/// use the radio-action "page.Lang" (with the language as target).
/// \param menu: Menu, where to add the language-entries to
/// \param shortcuts: Controller to add the shortcuts to (if any)
//-----------------------------------------------------------------------------
void PFilms::addLanguageMenus(Glib::RefPtr<Gio::Menu> menu, Glib::RefPtr<Gtk::ShortcutController> shortcuts) {
    TRACE9("PFilms::addLanguageMenus (Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gtk::ShortcutController>)");

    addMenuEntry(menu, _("_Original name"), "page.Lang::", "<ctl>0", shortcuts);

    std::string accel("<ctl>1");
    for (const auto& [code, language] : std::ranges::subrange(Language::begin(), Language::end())) {
        TRACE9("PFilms::addLanguageMenus (...) - Adding language " << code);
        if (accel[5] <= '9') {
            addMenuEntry(menu, language.getInternational(), "page.Lang::" + code, accel, shortcuts);
            ++accel[5];
        }
        else
            addMenuEntry(menu, language.getInternational(), "page.Lang::" + code);
    }
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param menuEntries: Edit-menu to add the page-specific entries to
/// \param menuOther: Menu to add further top-level menus to
/// \param grpAction: Action-group to add the actions to
/// \param shortcuts: Controller to add the keyboard shortcuts to
//-----------------------------------------------------------------------------
void PFilms::addMenu(Glib::RefPtr<Gio::Menu> menuEntries, Glib::RefPtr<Gio::Menu> menuOther,
                     Glib::RefPtr<Gio::SimpleActionGroup> grpAction, Glib::RefPtr<Gtk::ShortcutController> shortcuts) {
    TRACE9("PFilms::addMenu (...)");
    contract_assert(!imgLang);
    imgLang = std::make_unique<LanguageImg>(Film::currLang.c_str());
    imgLang->set_margin(5);
    imgLang->signal_clicked().connect(sigc::mem_fun(*this, &PFilms::selectLanguage));
    addStatusWidget(*imgLang);

    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    apMenus[UNDO] = grpAction->add_action("FUndo", sigc::mem_fun(*this, &PFilms::undo));
    addMenuEntry(sec, _("_Undo"), "page.FUndo", _("<ctl>Z"), shortcuts);
    menuEntries->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[NEW1] = grpAction->add_action("NDirector", sigc::mem_fun(*this, &PFilms::newDirector));
    addMenuEntry(sec, _("New _director"), "page.NDirector", _("<ctl>N"), shortcuts);
    apMenus[NEW2] = grpAction->add_action("NFilm", sigc::mem_fun(*this, &PFilms::newFilm));
    addMenuEntry(sec, _("_New film"), "page.NFilm", _("<ctl><alt>N"), shortcuts);
    menuEntries->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[DELETE] = grpAction->add_action("FDelete", sigc::mem_fun(*this, &PFilms::deleteSelection));
    addMenuEntry(sec, _("_Delete"), "page.FDelete", _("<ctl>Delete"), shortcuts);
    menuEdit = grpAction->add_action("FEdit", sigc::mem_fun(*this, &PFilms::editSelection));
    addMenuEntry(sec, _("_Edit"), "page.FEdit", _("<ctl>Return"), shortcuts);
    menuEntries->append_section(sec);

    sec = Gio::Menu::create();
    grpAction->add_action("FImport", sigc::mem_fun(*this, &PFilms::importFromIMDb));
    addMenuEntry(sec, _("_Import from IMDb.com ..."), "page.FImport", _("<ctl>I"), shortcuts);
    grpAction->add_action("FImportDescr", sigc::mem_fun(*this, &PFilms::importInfoFromIMDb));
    addMenuEntry(sec, _("_Import information from IMDb.com ..."), "page.FImportDescr", _("<shft><ctl>I"), shortcuts);
    menuEntries->append_section(sec);

    // Language-menu (as radio-action)
    menuLang = grpAction->add_action_radio_string(
        "Lang",
        [this](const Glib::ustring& lang) {
            menuLang->change_state(lang);
            changeLanguage(lang);
        },
        Film::currLang);

    Glib::RefPtr<Gio::Menu> menuLanguages(Gio::Menu::create());
    addLanguageMenus(menuLanguages, shortcuts);
    menuOther->append_submenu(_("_Language"), menuLanguages);

    // Popup-menu when clicking on the language-image
    Glib::RefPtr<Gio::Menu> menuPopup(Gio::Menu::create());
    addLanguageMenus(menuPopup);
    popLang = std::make_unique<Gtk::PopoverMenu>(menuPopup);
    popLang->set_parent(*imgLang);

    apMenus[UNDO]->set_enabled(false);
    filmSelected();
}

//-----------------------------------------------------------------------------
/// Removes page-related menus
//-----------------------------------------------------------------------------
void PFilms::removeMenu() {
    TRACE9("PFilms::removeMenu ()");
    if (popLang) {
        popLang->unparent();
        popLang.reset();
    }
    if (imgLang) {
        removeStatusWidget(*imgLang);
        imgLang.reset();
    }
}

//-----------------------------------------------------------------------------
/// Callback after selecting menu to set the language in which the
/// films are displayed
/// \param lang: Lanuage in which the films should be displayed
//-----------------------------------------------------------------------------
void PFilms::changeLanguage(const std::string& lang) {
    TRACE1("PFilms::changeLanguage (const std::string&) - " << lang);
    if (lang != Film::currLang)
        setLanguage(lang);
}

//-----------------------------------------------------------------------------
/// Changes the language in which the films are displayed
/// \param lang: Lanuage in which the films should be displayed
//-----------------------------------------------------------------------------
void PFilms::setLanguage(const std::string& lang) {
    TRACE9("PFilms::setLanguage (const std::string&) - " << lang);
    Film::currLang = lang;
    if ((lang.size() == 2) && !loadedLangs[lang])
        loadData(lang);

    films.update(lang);
    if (imgLang)
        imgLang->update(lang.c_str());
}

//-----------------------------------------------------------------------------
/// Callback to select the language in which to display the films
//-----------------------------------------------------------------------------
void PFilms::selectLanguage() {
    TRACE9("PFilms::selectLanguage ()");

    contract_assert(popLang);
    popLang->popup();
}

//-----------------------------------------------------------------------------
/// Sets the focus to the film-list
//-----------------------------------------------------------------------------
void PFilms::getFocus() { films.grab_focus(); }

//-----------------------------------------------------------------------------
/// Finds the film with the passed id
/// \param directors: Vector of known directors
/// \param relFilms: Relation of above directors to their films
/// \param id: Id of film to find
/// \returns HFilm: Found film (undefined, if not found)
//-----------------------------------------------------------------------------
HFilm PFilms::findFilm(const std::vector<HDirector>& directors, const YGP::Relation1_N<HDirector, HFilm>& relFilms,
                       unsigned int id) {
    for (const auto& director : directors) {
        contract_assert(director);
        for (const auto& film : relFilms.getObjects(director)) {
            contract_assert(film);
            if (film->getId() == id)
                return film;
        }
    }
    return {};
}

//-----------------------------------------------------------------------------
/// Loads the films from the database.
//-----------------------------------------------------------------------------
void PFilms::loadData() {
    TRACE9("PFilms::loadData ()");
    try {
        YGP::StatusObject stat;
        StorageFilm::loadDirectors(directors, stat);
        std::ranges::sort(directors, &Director::compByName);

        std::map<unsigned int, std::vector<HFilm>> aFilms;
        unsigned int cFilms(StorageFilm::loadFilms(aFilms, stat));
        TRACE8("PFilms::loadData () - Found " << cFilms << " films");

        for (const auto& hDirector : directors) {
            contract_assert(hDirector);
            Gtk::TreeModel::Row director(films.append(hDirector));

            if (auto iFilm(aFilms.find(hDirector->getId())); iFilm != aFilms.end()) {
                for (auto& film : iFilm->second) {
                    films.append(film, director);
                    relFilms.relate(hDirector, film);
                } // end-for all films for a director
                aFilms.erase(iFilm);
            } // end-if director has films
        } // end-for all directors

        films.expand_all();

        showStatus(Glib::ustring::compose(Glib::locale_to_utf8(ngettext("Loaded %1 film", "Loaded %1 films", cFilms)),
                                          Glib::ustring(YGP::ANumeric::toString(cFilms))));

        loaded = true;

        if (stat.getType() > YGP::StatusObject::UNDEFINED) {
            stat.generalize(_("Warnings loading films"));
            XGP::MessageDlg::create(stat);
        }
    }
    catch (std::exception& err) {
        showError(Glib::ustring::compose(_("Can't query available films!\n\nReason: %1"), err.what()));
    }
}

//-----------------------------------------------------------------------------
/// Loads the films from the database for a certain language.
///
/// According to the available information the page of the notebook
/// is created.
/// \param lang: Language for films to load
//-----------------------------------------------------------------------------
void PFilms::loadData(const std::string& lang) {
    TRACE5("PFilms::loadFilms (const std::string&) - Language: " << lang);
    try {
        StorageFilm::loadNames(directors, relFilms, lang);
        loadedLangs[lang] = true;
    }
    catch (std::exception& err) {
        showError(Glib::ustring::compose(_("Can't query available films!\n\nReason: %1"), err.what()));
    }
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PFilms::saveData() {
    TRACE9("PFilms::saveData ()");

    std::vector<HEntity> aSaved;
    std::vector<HEntity>::iterator posSaved(aSaved.end());

    // Save all data in the undo-buffer. Successfully saved data is removed
    // this buffer. This means sucessful saving disables undo
    while (!aUndo.empty()) {
        Undo last(aUndo.top());

        // Only save each entries once (when if it is changed more then once)
        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
        if ((posSaved == aSaved.end()) || (*posSaved != last.getEntity())) {
            switch (last.what()) {
            case FILM: {
                contract_assert(typeid(*last.getEntity()) == typeid(Film));
                HFilm film(std::dynamic_pointer_cast<Film>(last.getEntity()));
                if (last.how() == Undo::DELETE) {
                    if (film->getId()) {
                        contract_assert(film->getId() == last.column());
                        StorageFilm::deleteFilm(film->getId());
                    }

                    auto delRel(delRelation.find(last.getEntity()));
                    contract_assert(delRel != delRelation.end());
                    contract_assert(typeid(*delRel->second) == typeid(Director));
                    delRelation.erase(delRel);
                }
                else {
                    HDirector director(relFilms.getParent(film));
                    if (!director->getId()) {
                        contract_assert(!std::ranges::contains(aSaved, director));
                        contract_assert(!delRelation.contains(director));

                        SaveCelebrity::store(director, "Directors", *getWindow());
                        aSaved.insert(std::ranges::lower_bound(aSaved, director), director);
                        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
                    }
                    StorageFilm::saveFilm(film, relFilms.getParent(film)->getId());
                }
                break;
            }

            case DIRECTOR: {
                contract_assert(typeid(*last.getEntity()) == typeid(Director));
                HDirector director(std::dynamic_pointer_cast<Director>(last.getEntity()));
                if (last.how() == Undo::DELETE) {
                    if (director->getId()) {
                        contract_assert(director->getId() == last.column());
                        StorageFilm::deleteDirector(director->getId());
                    }
                }
                else
                    SaveCelebrity::store(director, "Directors", *getWindow());
                break;
            }

            default:
                contract_assert(false);
            } // end-switch
            aSaved.insert(posSaved, last.getEntity());
        }
        aUndo.pop();
    } // end-while
    contract_assert(apMenus[UNDO]);
    apMenus[UNDO]->set_enabled(false);

    contract_assert(delRelation.empty());
}

//-----------------------------------------------------------------------------
/// Removes the selected films or directors from the listbox. Depending films
/// are deleted too.
//-----------------------------------------------------------------------------
void PFilms::deleteSelection() {
    TRACE9("PFilms::deleteSelection()");

    Glib::RefPtr<Gtk::TreeSelection> selection(films.get_selection());
    while (!selection->get_selected_rows().empty()) {
        std::vector<Gtk::TreePath> list(selection->get_selected_rows());
        contract_assert(!list.empty());

        Gtk::TreeModel::iterator iter(films.get_model()->get_iter(list.front()));
        contract_assert(iter);
        if (iter->parent()) // A film is going to be deleted
            deleteFilm(iter);
        else { // A director is going to be deleted
            TRACE9("PFilms::deleteSelection() - Deleting " << iter->children().size() << " children");
            HDirector director(films.getDirectorAt(iter));
            contract_assert(director);
            while (!iter->children().empty()) {
                Gtk::TreeModel::iterator child(iter->children().begin());
                deleteFilm(child);
            }

            Gtk::TreePath path(films.getModel()->get_path(iter));
            aUndo.push(Undo(Undo::DELETE, DIRECTOR, director->getId(), director, path, ""));
            films.getModel()->erase(iter);
        }
    }
    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Deletes the passed film
/// \param film: Iterator to film to delete
//-----------------------------------------------------------------------------
void PFilms::deleteFilm(const Gtk::TreeModel::iterator& film) {
    HFilm hFilm(films.getFilmAt(film));
    TRACE9("PFilms::deleteFilm (const Gtk::TreeModel::iterator&) - Deleting film " << hFilm->getName());
    contract_assert(relFilms.isRelated(hFilm));
    HDirector hDirector(relFilms.getParent(hFilm));
    contract_assert(hDirector);

    contract_assert(!delRelation.contains(hFilm));

    Glib::RefPtr<Gtk::TreeStore> model(films.getModel());
    Gtk::TreePath path(model->get_path(films.getOwner(hDirector)));
    aUndo.push(Undo(Undo::DELETE, FILM, hFilm->getId(), hFilm, path, ""));
    delRelation[hFilm] = hDirector;

    relFilms.unrelate(hDirector, hFilm);
    model->erase(film);
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
/// \param lang: Language, in which to export
//-----------------------------------------------------------------------------
void PFilms::export2HTML(unsigned int fd, const std::string& lang) {
    TRACE1("PFilms::export2HTML (unsinged int) - Writing: " << lang);
    std::string oldLang(Film::currLang);
    Film::currLang = lang;

    // Load the names of the films in the actual language
    if (!loadedLangs[Film::currLang])
        loadData(Film::currLang);
    contract_assert(loadedLangs.contains(Film::currLang) && loadedLangs.at(Film::currLang));

    std::ranges::sort(directors, &Director::compByName);

    // Write film-information
    for (const auto& director : directors)
        if (relFilms.isRelated(director)) {
            std::stringstream output;
            output << 'D' << *director;

            const std::vector<HFilm>& dirFilms(relFilms.getObjects(director));
            contract_assert(!dirFilms.empty());
            for (const auto& film : dirFilms)
                output << 'M' << *film;

            const std::string data(output.str());
            TRACE9("PFilms::export2HTML (unsinged int) - Writing: " << data);
            if (::write(fd, data.data(), data.size()) != static_cast<ssize_t>(data.size())) {
                showError(Glib::ustring::compose(_("Couldn't write data!\n\nReason: %1"), std::strerror(errno)),
                          _("Error exporting films to HTML!"));
                break;
            }
        }

    Film::currLang = oldLang;
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PFilms::undo() {
    TRACE1("PFilms::undo ()");
    contract_assert(!aUndo.empty());

    Undo last(aUndo.top());
    switch (last.what()) {
    case FILM:
        undoFilm(last);
        break;

    case DIRECTOR:
        undoDirector(last);
        break;

    default:
        contract_assert(false);
    } // end-switch

    aUndo.pop();
    if (aUndo.empty()) {
        enableSave(false);
        apMenus[UNDO]->set_enabled(false);
    }
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a film
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PFilms::undoFilm(const Undo& last) {
    TRACE5("PFilms::undoFilm(const Undo&)");

    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(films.getModel()->get_iter(path));

    contract_assert(typeid(*last.getEntity()) == typeid(Film));
    HFilm film(std::dynamic_pointer_cast<Film>(last.getEntity()));
    TRACE9("PFilms::undoFilm(const Undo&) - " << last.how() << ": " << film->getName());

    switch (last.how()) {
    case Undo::CHANGED:
        contract_assert(iter->parent());

        switch (last.column()) {
        case 0:
            film->setName(last.getValue());
            break;

        case 1:
            film->setYear(last.getValue());
            break;

        case 2:
            film->setGenre(static_cast<unsigned int>(last.getValue()[0]));
            break;

        case 3:
            film->setType(last.getValue());
            break;

        case 4:
            film->setLanguage(last.getValue());
            break;

        case 5:
            film->setTitles(last.getValue());
            break;

        case 99:
            film->setDescription(last.getValue());
            film->setImage("");
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;

    case Undo::INSERT:
        contract_assert(iter->parent());
        contract_assert(relFilms.isRelated(film));
        relFilms.unrelate(films.getDirectorAt(iter->parent()), film);
        films.getModel()->erase(iter);
        iter = films.getModel()->children().end();
        break;

    case Undo::DELETE: {
        auto delRel(delRelation.find(last.getEntity()));
        contract_assert(typeid(*delRel->second) == typeid(Director));
        HDirector director(std::dynamic_pointer_cast<Director>(delRel->second));
        Gtk::TreeRow rowDirector(*films.getOwner(director));

        iter = films.append(film, rowDirector).get_iter();
        path = films.getModel()->get_path(iter);

        relFilms.relate(director, film);

        delRelation.erase(delRel);
        break;
    }

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        films.update(row);
    }
    films.set_cursor(path);
    films.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a director
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PFilms::undoDirector(const Undo& last) {
    TRACE5("PFilms::undoDirector (const Undo&)");

    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(films.getModel()->get_iter(path));

    contract_assert(last.getEntity());
    contract_assert(typeid(*last.getEntity()) == typeid(Director));
    HDirector director(std::dynamic_pointer_cast<Director>(last.getEntity()));
    TRACE9("PFilms::undoDirector (const Undo&) - " << last.how() << ": " << director->getName());

    switch (last.how()) {
    case Undo::CHANGED:
        contract_assert(iter);
        contract_assert(!iter->parent());

        switch (last.column()) {
        case 0:
            director->setName(last.getValue());
            break;

        case 1:
            director->setLifespan(last.getValue());
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;

    case Undo::INSERT:
        contract_assert(iter);
        contract_assert(!iter->parent());
        contract_assert(!relFilms.isRelated(director));
        films.getModel()->erase(iter);
        iter = films.getModel()->children().end();
        break;

    case Undo::DELETE:
        if (iter)
            contract_assert(!iter->parent());
        else
            iter = films.getModel()->children().end();
        iter = films.insert(director, iter).get_iter();
        path = films.getModel()->get_path(iter);
        break;

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        films.update(row);
    }
    films.set_cursor(path);
    films.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PFilms::clear() {
    relFilms.unrelateAll();
    directors.clear();
    films.clear();
    films.getModel()->clear();
    NBPage::clear();
    loadedLangs.clear();
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for a film from IMDb.com
//-----------------------------------------------------------------------------
void PFilms::importFromIMDb() {
    TRACE9("PFilms::importFromIMDb ()");
    ImportFromIMDb* dlg(ImportFromIMDb::create());
    dlg->sigLoaded.connect(sigc::mem_fun(*this, &PFilms::importFilm));
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to import information for films from IMDb.com
//-----------------------------------------------------------------------------
void PFilms::importInfoFromIMDb() {
    TRACE9("PFilms::importDescriptionFromIMDb ()");
    // Ownership is passed along the import-chain; importNextFilm frees the list at the end
    auto* iFilms(new std::vector<HFilm>);
    for (const auto& director : directors)
        for (const auto& film : relFilms.getObjects(director))
            if (film->getDescription().empty() || film->getImage().empty())
                iFilms->push_back(film);
    TRACE5("PFilms::importDescriptionFromIMDb () - To process: " << iFilms->size());

    ImportFromIMDb* dlg(ImportFromIMDb::create());
    // dlg->sigLoaded.connect (bind (mem_fun (*this, &PFilms::continousImportFilm), dlg, iFilms));
    importNextFilm(dlg, iFilms);
}

//-----------------------------------------------------------------------------
/// Imports the last entry of the passed list of films
/// \param dlg Dialog displaying the import-information
/// \param films Pointer to list of all films to import
//-----------------------------------------------------------------------------
void PFilms::importNextFilm(ImportFromIMDb* dlg, std::vector<HFilm>* films) {
    if (!films->empty()) {
        TRACE5("PFilms::importNextFilm (ImportFromIMDb*, std::vector<HFilm>*) - " << films->back()->getName());
        dlg->searchFor(films->back()->getName(""));
    }
    else {
        delete films;
        delete dlg;
    }
}

//-----------------------------------------------------------------------------
/// Imports the passed film-information and adds them appropiately to the
/// list of films.
/// Algorithm: If the name of the director does exist, ask if they are equal.
///    The film is added to the respective director
/// \param director Name of the director
/// \param film Name of the film with year in parenthesis at the end
/// \param genre Genre of the film
/// \param summary Synopsis of the film
/// \param image Poster of the film
/// \param dlg Dialog to load elements
/// \param filmlist List of films to load
//-----------------------------------------------------------------------------
bool PFilms::continousImportFilm(const Glib::ustring& director, [[maybe_unused]] const Glib::ustring& film,
                                 [[maybe_unused]] const Glib::ustring& genre,
                                 const Glib::ustring& summary, const std::string& image, ImportFromIMDb* dlg,
                                 std::vector<HFilm>* filmlist) {
    TRACE3("PFilms::continousImportFilm (4x const Glib::ustring&, const std::string&, std::vector<HFilm>*) - "
           << filmlist->back()->getName() << '/' << image.size());

    auto last(std::prev(filmlist->end()));
    TRACE5("PFilms::continousImportFilm (4x const Glib::ustring&, const std::string&, std::vector<HFilm>*) - "
           << (*last)->getName() << "->" << relFilms.getParent(*last)->getName());
    if (director.empty() || (director == relFilms.getParent(*last)->getName())) {
        bool changed(false);
        if ((*last)->getDescription().empty() && !summary.empty()) {
            (*last)->setDescription(summary);
            changed = true;
        }
        if ((*last)->getImage().empty() && !image.empty()) {
            (*last)->setImage(image);
            changed = true;
        }
        if (changed) {
            const Gtk::TreeModel::iterator row(films.getObject(*last));
            Glib::ustring empty;
            filmChanged(row, 99, empty);
        }
    }
    else {
        const Glib::ustring msg(
            Glib::ustring::compose(_("Not setting data from IMDb for '%1' as the directors differ (found %2, should be %3)!"),
                                   (*last)->getName(), director, relFilms.getParent(*last)->getName()));
        Gtk::MessageDialog dlgMsg(*dlg, msg, false, Gtk::MessageType::INFO, Gtk::ButtonsType::OK_CANCEL);
        if (XGP::runModal(dlgMsg) == Gtk::ResponseType::OK)
            return false;
    }

    filmlist->erase(last);
    Glib::signal_idle().connect_once([this, dlg, filmlist]() { importNextFilm(dlg, filmlist); });
    return false;
}

//-----------------------------------------------------------------------------
/// Imports the passed film-information and adds them appropiately to the
/// list of films.
/// Algorithm: If the name of the director does exist, ask if they are equal.
///    The film is added to the respective director
/// \param director Name of the director
/// \param film Name of the film with year in parenthesis at the end
/// \param genre Genre of the film
/// \param summary Synopsis of the film
/// \param image Poster of the film
//-----------------------------------------------------------------------------
bool PFilms::importFilm(const Glib::ustring& director, const Glib::ustring& film, const Glib::ustring& genre,
                        const Glib::ustring& summary, const std::string& image) {
    TRACE5("PFilms::importFilm (4x const Glib::ustring&, const std::string&) - " << director << ": " << film);

    Glib::ustring nameFilm(film);
    YGP::AYear year;

    // Strip trailing type of film (IMDb adds " (x)" for certain types of films)
    if ((nameFilm.size() > 4) && (nameFilm[nameFilm.size() - 1] == ')') && (nameFilm[nameFilm.size() - 3] == '(') &&
        (nameFilm[nameFilm.size() - 4] == ' '))
        nameFilm = nameFilm.substr(0, nameFilm.size() - 4);

    // Check if the film has the year in parenthesis appended
    std::string::size_type pos(nameFilm.rfind(" (", nameFilm.size() - 2));
    if ((nameFilm[nameFilm.size() - 1] == ')') && (pos != std::string::npos)) {
        try {
            year = nameFilm.substr(pos + 2, nameFilm.size() - pos - 3);
            nameFilm = nameFilm.substr(0, pos);
        }
        catch (const std::invalid_argument&) {
        }
    }

    // Create the new director
    auto hDirector(std::make_shared<Director>());
    hDirector->setName(director);

    Gtk::TreeModel::iterator iNewDirector(films.getOwner(director));
    if (iNewDirector) {
        std::vector<HDirector> sameDirectors;
        std::map<unsigned long, Gtk::TreeModel::iterator> iDirectors;

        // Get all directors with a similar name
        for (auto i(films.getModel()->children().begin()); i != films.getModel()->children().end(); ++i) {
            HDirector actDirector(films.getDirectorAt(i));
            if (!actDirector->getName().compare(0, director.size(), director)) {
                iDirectors[actDirector->getId()] = i;
                sameDirectors.push_back(actDirector);
            }
        }

        auto* win(dynamic_cast<Gtk::Window*>(getWindow()->get_root()));
        contract_assert(win);
        std::unique_ptr<SaveCelebrity> dlgCeleb(SaveCelebrity::create(*win, hDirector, sameDirectors));
        switch (dlgCeleb->run()) {
        case Gtk::ResponseType::YES:
            TRACE9("PFilms::importFilm (3x const Glib::ustring&) - Same director " << dlgCeleb->getIdOfSelection());
            contract_assert(dlgCeleb->getIdOfSelection());
            hDirector->setId(dlgCeleb->getIdOfSelection());
            iNewDirector = iDirectors[dlgCeleb->getIdOfSelection()];
            break;

        case Gtk::ResponseType::NO:
            iNewDirector = addDirector(hDirector);
            break;

        default:
            return false;
        }
    }
    else
        iNewDirector = addDirector(hDirector);

    int idGenre(films.getGenre(genre));
    if (idGenre == -1)
        idGenre = 0;

    auto hFilm(std::make_shared<Film>());
    hFilm->setGenre(idGenre);
    hFilm->setName(nameFilm);
    hFilm->setYear(year);
    hFilm->setDescription(summary);
    hFilm->setImage(image);
    addFilm(hFilm, iNewDirector);
    return true;
}

//-----------------------------------------------------------------------------
/// Callback when trying to display a tooltip
/// \param x Position of tooltip (X-axis)
/// \param y Position of tooltip (Y-axis)
/// \param keyboard Flag if caused by a keyboard-action
/// \param tooltip Tooltip widget; will be updated
//-----------------------------------------------------------------------------
bool PFilms::onQueryTooltip(int x, int y, bool keyboard, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
    Gtk::TreeModel::iterator iter;
    if (films.get_tooltip_context_iter(x, y, keyboard, iter)) {
        if (iter->parent()) {
            HFilm film(films.getFilmAt(iter));
            Glib::ustring summary(film->getDescription());
            if (!summary.empty())
                tooltip->set_text(summary);

            std::string image(film->getImage());
            if (!image.empty()) {
                try {
                    tooltip->set_icon(Gdk::Texture::create_from_bytes(Glib::Bytes::create(image.data(), image.size())));
                }
                catch (const Glib::Error&) {
                    image.clear();
                }
            }
            return !summary.empty() || !image.empty();
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Edits the selected films or directors from the listbox.
//-----------------------------------------------------------------------------
void PFilms::editSelection() {
    TRACE9("PFilms::editSelection()");

    Glib::RefPtr<Gtk::TreeSelection> filmSel(films.get_selection());
    contract_assert(filmSel);
    Gtk::TreeModel::iterator iter(filmSel->get_selected());
    contract_assert(iter);
    HFilm film(films.getFilmAt(iter));

    FilmDataEditor dlg;
    dlg.setSummary(film->getDescription());
    dlg.setIcon(film->getImage());
    if (auto* win(dynamic_cast<Gtk::Window*>(getWindow()->get_root())); win)
        dlg.set_transient_for(*win);
    if (XGP::runModal(dlg) == Gtk::ResponseType::OK) {
        Glib::ustring summary(film->getDescription());
        filmChanged(iter, 99, summary);
        film->setDescription(dlg.getSummary());
        if (const std::string icon(dlg.getIcon()); !icon.empty())
            film->setImage(icon);
    }
}
