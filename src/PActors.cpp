// PROJECT     : CDManager
// SUBSYSTEM   : Actors
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 20.01.2006
// COPYRIGHT   : Copyright (C) 2006, 2009 - 2011, 2026

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
#include <memory>

#include <glibmm/convert.h>
#include <glibmm/bytes.h>

#include <gdkmm/texture.h>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>

#include "FilmList.h"
#include "Genres.h"
#include "PFilms.h"
#include "SaveCeleb.h"
#include "StorageActor.h"

#include "PActors.h"

//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling actors
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
/// \param films: Reference to film-page
//-----------------------------------------------------------------------------
PActors::PActors(Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave, const Genres& genres, PFilms& films)
    : NBPage(status, menuSave), actors(genres), relActors("actors"), films(films) {
    TRACE9("PActors::PActors (Gtk::Statusbar&, Glib::RefPtr<Gio::SimpleAction>, const Genres&, PFilms&)");

    auto* scrl(Gtk::make_managed<Gtk::ScrolledWindow>());
    scrl->set_has_frame(true);
    scrl->set_child(actors);
    scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

    Glib::RefPtr<Gtk::TreeSelection> sel(actors.get_selection());
    sel->signal_changed().connect(sigc::mem_fun(*this, &PActors::actorSelected));
    actors.signalActorChanged.connect(sigc::mem_fun(*this, &PActors::actorChanged));
    actors.set_has_tooltip();
    actors.signal_query_tooltip().connect(sigc::mem_fun(*this, &PActors::onQueryTooltip), true);

    widget = scrl;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PActors::~PActors() = default;

//-----------------------------------------------------------------------------
/// Callback after selecting an actor
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PActors::actorSelected() {
    TRACE9("PActords::actorSelected ()");
    contract_assert(actors.get_selection());

    Gtk::TreeModel::iterator s(actors.get_selection()->get_selected());
    if (actView) {
        enableEdit((s && s->parent()) ? OWNER_SELECTED : NONE_SELECTED);
        apMenus[NEW1]->set_enabled(false);
    }
    else {
        enableEdit(s ? OWNER_SELECTED : NONE_SELECTED);
        if (s && s->parent())
            apMenus[DELETE]->set_enabled(false);
    }
}

//----------------------------------------------------------------------------
/// Callback when changing an actor
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PActors::actorChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE9("PActors::actorChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&)");

    Gtk::TreePath path(actors.get_model()->get_path(row));
    HEntity entity(actors.getEntityAt(row));
    aUndo.push(Undo(Undo::CHANGED, ACTOR, column, entity, path, oldValue));

    if (actView)
        changeAllEntries(entity, actors.getModel()->children().begin(), actors.getModel()->children().end());
    std::ranges::sort(aActors, &Actor::compByName);

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Adds a new actor to the list
//-----------------------------------------------------------------------------
void PActors::newActor() {
    auto actor(std::make_shared<Actor>());
    Gtk::TreeModel::iterator i;
    if (actView) {
        contract_assert(actors.get_selection()->get_selected());
        i = actors.append(actor, actors.get_selection()->get_selected()).get_iter();
    }
    else
        i = actors.append(actor).get_iter();
    aActors.insert(std::ranges::lower_bound(aActors, actor, &Actor::compByName), actor);

    Gtk::TreePath path(actors.get_model()->get_path(i));
    actors.selectRow(i);
    actors.set_cursor(path, *actors.get_column(0), true);

    aUndo.push(Undo(Undo::INSERT, ACTOR, 0, actor, path, ""));

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Displays a dialog enabling to connect films and actors
//-----------------------------------------------------------------------------
void PActors::actorPlaysInFilm() {
    TRACE9("PActors::actorPlaysInFilm ()");
    contract_assert(actors.get_selection());
    Gtk::TreeModel::iterator p(actors.get_selection()->get_selected());
    contract_assert(p);

    HActor actor(std::dynamic_pointer_cast<Actor>(actors.getEntityAt(p)));
    if (!actor) {
        contract_assert(p->parent());
        p = p->parent();
        actor = std::dynamic_pointer_cast<Actor>(actors.getEntityAt(p));
    }
    contract_assert(actor);
    TRACE9("void PActors::actorPlaysInFilm () - Found actor " << actor->getName());

    RelateFilm* dlg(relActors.isRelated(actor)
                        ? RelateFilm::create(actor, relActors.getObjects(actor), films.getFilmList().getModel())
                        : RelateFilm::create(actor, films.getFilmList().getModel()));
    if (auto* win(dynamic_cast<Gtk::Window*>(actors.get_root())); win)
        dlg->set_transient_for(*win);
    dlg->signalRelateFilms.connect(sigc::mem_fun(*this, &PActors::relateFilms));
}

//-----------------------------------------------------------------------------
/// Relates films with an actor
/// \param actor: Actor, to which store films
/// \param films: Films, starring this actor
//-----------------------------------------------------------------------------
void PActors::relateFilms(const HActor& actor, const std::vector<HFilm>& films) {
    TRACE9("PActors::relateFilms (const HActor&, const std::vector<HFilm>&)");

    Glib::RefPtr<Gtk::TreeStore> model(actors.getModel());
    Gtk::TreeModel::iterator i(actors.findEntity(actor));
    contract_assert(i);
    Gtk::TreePath path(model->get_path(i));

    // Unrelate old films and relate with newly selected ones
    auto hOldRel(std::make_shared<RelUndo>());
    if (relActors.isRelated(actor))
        hOldRel->setRelatedFilms(relActors.getObjects(actor));

    aUndo.push(Undo(Undo::CHANGED, FILMS, actor->getId(), hOldRel, path, ""));
    delRelation[hOldRel] = actor;

    showFilms(actor, films);

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Shows the films of a certain actor
/// \param actor: Actor
/// \param newFilms: Array with new films
//-----------------------------------------------------------------------------
void PActors::showFilms(const HActor& actor, const std::vector<HFilm>& newFilms) {
    Glib::RefPtr<Gtk::TreeStore> model(actors.getModel());
    Gtk::TreeModel::iterator i(actors.findEntity(actor));
    contract_assert(i);

    // Unrelate old films from actor; in view-by-film mode this means also
    // remove the actors from the films
    TRACE5("PActors::showFilms (HActor, std::vector<HFilm>) - Relate to " << newFilms.size() << " films");
    std::vector<Gtk::TreeModel::iterator> emptyFilms;
    if (relActors.isRelated(actor)) {
        if (actView) {
            for (const auto& _ : relActors.getObjects(actor)) {
                Gtk::TreeModel::iterator iEntry(actors.findEntity(actor, 2));
                contract_assert(iEntry);
                contract_assert(iEntry->parent());
                Gtk::TreeModel::iterator parent(iEntry->parent());
                contract_assert(parent);
                model->erase(iEntry);

                // Check if parent of deleted actor has now no more childs;
                // If so, store this film, to maybe remove it later from the list
                if (parent->children().empty())
                    emptyFilms.push_back(parent);
            }
        }
        else
            while (!i->children().empty())
                model->erase(i->children().begin());
        relActors.unrelateAll(actor);
    }

    // Then relate the new (undone) films with actor; in view-by-film mode
    // this means also showing the actor for the films
    for (const auto& film : newFilms) {
        relActors.relate(actor, film);

        if (actView) {
            Gtk::TreeModel::iterator iter(actors.findEntity(film, 1));
            if (!iter)
                iter = actors.append(film).get_iter();

            actors.append(actor, iter);
            actors.expand_row(model->get_path(iter), true);
        }
        else
            actors.append(film, *i);
    }

    if (actView) {
        // Remove all films without actors
        for (const auto& film : emptyFilms)
            if (film->children().empty())
                model->erase(film);
    }
    else {
        // In view-by-actor mode the remove/append can be done in one step
        actors.selectRow(i);
        actors.expand_row(model->get_path(i), true);
    }
}

//-----------------------------------------------------------------------------
/// Loads the actors from the database
///
/// According to the available information the page of the notebook
/// is created.
//-----------------------------------------------------------------------------
void PActors::loadData() {
    TRACE5("PActors::loadData ()");
    if (!films.isLoaded())
        films.loadData();

    try {
        YGP::StatusObject stat;
        StorageActor::loadActors(aActors, stat);
        TRACE9("PActors::loadData () - Actors: " << aActors.size());

        if (!aActors.empty()) {
            std::ranges::sort(aActors, &Actor::compByName);

            std::map<unsigned int, std::vector<unsigned int>> actorFilms;
            StorageActor::loadActorsInFilms(actorFilms);

            // Iterate over all actors
            for (const auto& hActor : aActors) {
                contract_assert(hActor);
                Gtk::TreeModel::Row actor(actors.append(hActor));

                // Get the films the actor played in
                if (auto iActor(actorFilms.find(hActor->getId())); iActor != actorFilms.end()) {
                    // Get films to the film-IDs
                    std::vector<HFilm> films;
                    films.reserve(iActor->second.size());
                    for (const auto idFilm : iActor->second) {
                        if (HFilm film(findFilm(idFilm)); film)
                            films.push_back(film);
                        else
                            throw std::invalid_argument(
                                Glib::ustring::compose(_("The database contains an invalid reference (%1) to a film!"),
                                                       Glib::ustring(YGP::ANumeric::toString(idFilm))));
                    }

                    // Add the films to the actor
                    std::ranges::sort(films, Film::compByName);
                    for (const auto& film : films) {
                        contract_assert(film);
                        actors.append(film, actor);
                        relActors.relate(hActor, film);
                    } // end-for all films for an actor
                    actorFilms.erase(iActor);
                } // end-if director has actors for films
            } // end-for all actors
            actors.expand_all();
        } // endif actors available

        showStatus(Glib::ustring::compose(Glib::locale_to_utf8(ngettext("Loaded %1 actor", "Loaded %1 actors", aActors.size())),
                                          Glib::ustring(YGP::ANumeric::toString(aActors.size()))));

        loaded = true;

        if (stat.getType() > YGP::StatusObject::UNDEFINED) {
            stat.generalize(_("Warnings loading actors!"));
            XGP::MessageDlg::create(stat);
        }
    }
    catch (const std::exception& err) {
        showError(Glib::ustring::compose(_("Can't query the actors1!\n\nReason: %1"), err.what()));
    }
}

//-----------------------------------------------------------------------------
/// Sets the focus to the actor-list
//-----------------------------------------------------------------------------
void PActors::getFocus() { actors.grab_focus(); }

//-----------------------------------------------------------------------------
/// Finds the film with the passed id
/// \param id: Id of film to find
/// \returns HFilm: Found film (undefined, if not found)
//-----------------------------------------------------------------------------
HFilm PActors::findFilm(unsigned int id) const { return PFilms::findFilm(films.getDirectors(), films.getRelFilms(), id); }

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param menuEdit: Edit-menu to add the page-specific entries to
/// \param menuOther: Menu to add further top-level menus to
/// \param grpAction: Action-group to add the actions to
/// \param shortcuts: Controller to add the keyboard shortcuts to
//-----------------------------------------------------------------------------
void PActors::addMenu(Glib::RefPtr<Gio::Menu> menuEdit, Glib::RefPtr<Gio::Menu> menuOther,
                      Glib::RefPtr<Gio::SimpleActionGroup> grpAction, Glib::RefPtr<Gtk::ShortcutController> shortcuts) {
    // Add edit-menu
    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    apMenus[UNDO] = grpAction->add_action("AUndo", sigc::mem_fun(*this, &PActors::undo));
    addMenuEntry(sec, _("_Undo"), "page.AUndo", _("<ctl>Z"), shortcuts);
    menuEdit->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[NEW1] = grpAction->add_action("NActor", sigc::mem_fun(*this, &PActors::newActor));
    addMenuEntry(sec, _("New _actor"), "page.NActor", _("<ctl>N"), shortcuts);
    apMenus[NEW2] = grpAction->add_action("AddFilm", sigc::mem_fun(*this, &PActors::actorPlaysInFilm));
    addMenuEntry(sec, _("_Plays in film..."), "page.AddFilm", _("<ctl><alt>N"), shortcuts);
    menuEdit->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[DELETE] = grpAction->add_action("ADelete", sigc::mem_fun(*this, &PActors::deleteSelection));
    addMenuEntry(sec, _("_Delete"), "page.ADelete", _("<ctl>Delete"), shortcuts);
    menuEdit->append_section(sec);

    apMenus[UNDO]->set_enabled(false);

    // Add view-menu (as radio-action)
    contract_assert(actView < 2);
    menuView =
        grpAction->add_action_radio_string("View", sigc::mem_fun(*this, &PActors::changeView), actView ? "ByFilm" : "ByActor");
    Glib::RefPtr<Gio::Menu> menuViews(Gio::Menu::create());
    addMenuEntry(menuViews, _("By _actor"), "page.View::ByActor", "<ctl>1", shortcuts);
    addMenuEntry(menuViews, _("By _film"), "page.View::ByFilm", "<ctl>2", shortcuts);
    menuOther->append_submenu(_("_View"), menuViews);

    actView ? viewByFilm() : viewByActor();
}

//-----------------------------------------------------------------------------
/// Callback after selecting a view in the menu
/// \param view: Selected view ("ByActor" or "ByFilm")
//-----------------------------------------------------------------------------
void PActors::changeView(const Glib::ustring& view) {
    TRACE9("PActors::changeView (const Glib::ustring&) - " << view);
    contract_assert(menuView);
    menuView->change_state(view);
    (view == "ByFilm") ? viewByFilm() : viewByActor();
}

//-----------------------------------------------------------------------------
/// Removes the selected actor from the listbox. Depending films are disconnected.
//-----------------------------------------------------------------------------
void PActors::deleteSelection() {
    TRACE9("PActors::deleteSelection ()");

    Glib::RefPtr<Gtk::TreeSelection> selection(actors.get_selection());
    Gtk::TreeModel::iterator selRow(selection->get_selected());
    Glib::RefPtr<Gtk::TreeStore> model(actors.getModel());

    if (selRow) {
        contract_assert(!selRow->parent());
        Gtk::TreePath path(model->get_path(selRow));
        HActor actor(std::dynamic_pointer_cast<Actor>(actors.getEntityAt(selRow)));
        contract_assert(actor);
        TRACE9("PActors::deleteSelectedActor () - Deleting " << actor->getName());

        auto hOldRel(std::make_shared<RelUndo>());
        if (relActors.isRelated(actor)) {
            hOldRel->setRelatedFilms(relActors.getObjects(actor));
            relActors.unrelateAll(actor);

            while (!selRow->children().empty())
                model->erase(selRow->children().begin());
        }

        delRelation[hOldRel] = actor;
        aUndo.push(Undo(Undo::DELETE, FILMS, actor->getId(), hOldRel, path, ""));
        aUndo.push(Undo(Undo::DELETE, ACTOR, actor->getId(), actor, path, ""));
        model->erase(selRow);
    }
    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PActors::undo() {
    TRACE4("PActors::undo () - " << aUndo.size());
    contract_assert(!aUndo.empty());

    Undo last(aUndo.top());
    TRACE9("PActors::undo () - Undoing " << last.what() << ": " << last.how());
    switch (last.what()) {
    case ACTOR:
        undoActor(last);
        break;

    case FILMS: {
        contract_assert((last.how() == Undo::CHANGED) || (last.how() == Undo::DELETE));
        contract_assert(typeid(*last.getEntity()) == typeid(RelUndo));
        auto relActor(std::dynamic_pointer_cast<RelUndo>(last.getEntity()));

        auto delRel(delRelation.find(last.getEntity()));
        contract_assert(typeid(*delRel->second) == typeid(Actor));
        HActor actor(std::dynamic_pointer_cast<Actor>(delRel->second));

        showFilms(actor, relActor->getRelatedFilms());
        delRelation.erase(delRel);
        break;
    }

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
/// Undoes the last changes to an actor
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PActors::undoActor(const Undo& last) {
    TRACE7("PActors::undoActor (const Undo&)");

    Glib::RefPtr<Gtk::TreeStore> model(actors.getModel());
    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(model->get_iter(path));

    contract_assert(typeid(*last.getEntity()) == typeid(Actor));
    HActor actor(std::dynamic_pointer_cast<Actor>(last.getEntity()));
    TRACE9("PActors::undoActor (const Undo&) - " << last.how() << ": " << actor->getName());

    switch (last.how()) {
    case Undo::CHANGED:
        switch (last.column()) {
        case 0:
            actor->setName(last.getValue());
            if (actView)
                changeAllEntries(actor, model->children().begin(), model->children().end());
            break;

        case 1:
            actor->setLifespan(last.getValue());
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;

    case Undo::INSERT:
        contract_assert(!relActors.isRelated(actor));
        model->erase(iter);
        iter = model->children().end();
        break;

    case Undo::DELETE:
        iter = actors.insert(actor, iter).get_iter();
        path = model->get_path(iter);
        break;

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        actors.update(row);
    }
    actors.set_cursor(path);
    actors.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PActors::clear() {
    aActors.clear();
    actors.clear();
    actors.getModel()->clear();
    relActors.unrelateAll();
    NBPage::clear();
    actView = 0;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PActors::saveData() {
    TRACE9("PActors::saveData ()");

    std::vector<HEntity> aSaved;
    auto posSaved(aSaved.end());

    while (!aUndo.empty()) {
        Undo last(aUndo.top());

        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
        if ((posSaved == aSaved.end()) || (*posSaved != last.getEntity())) {
            switch (last.what()) {
            case FILMS:
            case ACTOR: {
                HEntity entity(last.getEntity());
                if (last.what() == FILMS) {
                    auto rel(delRelation.find(entity));
                    if (rel == delRelation.end())
                        break; // Films have already been saved together with their actor
                    entity = rel->second;
                }
                HActor actor(std::dynamic_pointer_cast<Actor>(entity));
                if (last.how() == Undo::DELETE) {
                    if (actor->getId()) {
                        contract_assert(actor->getId() == last.column());
                        StorageActor::deleteActor(actor->getId());
                    }
                }
                else {
                    SaveCelebrity::store(actor, "Actors", *getWindow());

                    // Check if the related films have been changed
                    HEntity entityActor(actor);
                    for (auto i(delRelation.begin()); i != delRelation.end();)
                        if (i->second == entityActor) {
                            saveRelatedFilms(actor);
                            i = delRelation.erase(i);
                        }
                        else
                            ++i;
                }
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

    delRelation.clear();
}

//-----------------------------------------------------------------------------
/// Saves the related films
/// \param actor: Actor whose films shall be stored
/// \throw std::exception in case of an error
//-----------------------------------------------------------------------------
void PActors::saveRelatedFilms(const HActor& actor) {
    TRACE9("PActors::saveRelatedFilms (const HActor&)");
    StorageActor::startTransaction();
    StorageActor::deleteActorFilms(actor->getId());

    try {
        for (const auto& film : relActors.getObjects(actor))
            StorageActor::saveActorFilm(actor->getId(), film->getId());
        StorageActor::commitTransaction();
    }
    catch (const std::exception&) {
        StorageActor::abortTransaction();
        throw;
    }
}

//-----------------------------------------------------------------------------
/// Views the list sorted by actor
//-----------------------------------------------------------------------------
void PActors::viewByActor() {
    TRACE9("PActors::viewByActor () - Actors: " << aActors.size());
    actView = 0;

    contract_assert(actors.get_column(0));
    actors.get_column(0)->set_title(_("Actors/Films"));

    // Fill list sorted by actors with their films as child
    actors.clear();
    for (const auto& hActor : aActors) {
        Gtk::TreeModel::Row actor(actors.append(hActor));

        if (relActors.isRelated(hActor))
            for (const auto& film : relActors.getObjects(hActor))
                actors.append(film, actor);
    }
    actors.expand_all();
    actorSelected();
}

//-----------------------------------------------------------------------------
/// Views the list sorted by film
//-----------------------------------------------------------------------------
void PActors::viewByFilm() {
    actView = 1;

    contract_assert(actors.get_column(0));
    actors.get_column(0)->set_title(_("Films/Actors"));

    std::vector<HFilm> aFilms;
    for (const auto& hActor : aActors)
        if (relActors.isRelated(hActor))
            for (const auto& film : relActors.getObjects(hActor)) {
                auto pos(std::ranges::lower_bound(aFilms, film));
                if ((pos == aFilms.end()) || (*pos != film))
                    aFilms.insert(pos, film);
            }
    std::ranges::sort(aFilms, &Film::compByName);

    // Fill list sorted by actors with their films as child
    actors.clear();
    for (const auto& hFilm : aFilms) {
        Gtk::TreeModel::Row film(actors.append(hFilm));

        if (relActors.isRelated(hFilm))
            for (const auto& actor : relActors.getParents(hFilm))
                actors.append(actor, film);
    }
    actors.expand_all();
    actorSelected();
}

//-----------------------------------------------------------------------------
/// Updates all entries showing the passed entry
/// \param entry: Entry to find
/// \param begin: Start object
/// \param end: End object
//-----------------------------------------------------------------------------
void PActors::changeAllEntries(const HEntity& entry, Gtk::TreeModel::iterator begin, Gtk::TreeModel::iterator end) {
    while (begin != end) {
        Gtk::TreeModel::Row row(*begin);
        if (entry == actors.getEntityAt(row))
            actors.update(row);

        if (!begin->children().empty())
            changeAllEntries(entry, begin->children().begin(), begin->children().end());
        ++begin;
    } // end-while
}

//-----------------------------------------------------------------------------
/// Callback when trying to display a tooltip
/// \param x Position of tooltip (X-axis)
/// \param y Position of tooltip (Y-axis)
/// \param keyboard Flag if caused by a keyboard-action
/// \param tooltip Tooltip widget; will be updated
//-----------------------------------------------------------------------------
bool PActors::onQueryTooltip(int x, int y, bool keyboard, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
    Gtk::TreeModel::iterator iter;
    if (actors.get_tooltip_context_iter(x, y, keyboard, iter)) {
        if (HFilm film(std::dynamic_pointer_cast<Film>(actors.getEntityAt(iter))); film) {
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
