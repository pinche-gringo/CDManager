// PROJECT     : CDManager
// SUBSYSTEM   : Actor
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 30.09.2005
// COPYRIGHT   : Copyright (C) 2005 - 2007, 2009 - 2011, 2026

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
#include <typeinfo>

#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/window.h>

#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XValue.h>

#include "Actor.h"
#include "Film.h"

#include "ActorList.h"

//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
ActorList::ActorList(const Genres& genres) : genres(genres) {
    TRACE9("ActorList::ActorList (const Genres&)");
    mOwnerObjects = Gtk::TreeStore::create(colActors);

    set_model(mOwnerObjects);

    append_column(_("Actors/Films"), colActors.name);
    append_column(_("Year"), colActors.year);
    append_column(_("Genre"), colActors.genre);

    contract_assert(get_columns().size() == 3);
    const std::array index{colActors.name.index(), colActors.year.index()};
    for (unsigned int i(0); i < index.size(); ++i) {
        Gtk::TreeViewColumn* column(get_column(i));
        column->set_sort_column(index[i]);
        column->set_resizable();

        Gtk::CellRenderer* renderer(get_column_cell_renderer(i));
        contract_assert(renderer);
        contract_assert(typeid(*renderer) == typeid(Gtk::CellRendererText));
        auto* rText(dynamic_cast<Gtk::CellRendererText*>(renderer));
        column->add_attribute(rText->property_editable(), colActors.editable);
        rText->signal_edited().connect(
            [this, i](const Glib::ustring& path, const Glib::ustring& value) { valueChanged(path, value, i); });
    }

    mOwnerObjects->set_sort_func(colActors.name, sigc::mem_fun(*this, &ActorList::sortByName));
    mOwnerObjects->set_sort_func(colActors.year, sigc::mem_fun(*this, &ActorList::sortByYear));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ActorList::~ActorList() { TRACE9("ActorList::~ActorList ()"); }

//-----------------------------------------------------------------------------
/// Appends a line to the list
/// \param entity: Entity to add
/// \param pos: Position in model for insert
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeRow ActorList::insert(const HEntity& entity, const Gtk::TreeModel::iterator& pos) {
    TRACE7("ActorList::insert (const HEntity&, const Gtk::TreeModel::iterator&)");

    Gtk::TreeRow newRow(*mOwnerObjects->insert(pos));
    newRow[colActors.entry] = entity;
    update(newRow);
    return newRow;
}

//-----------------------------------------------------------------------------
/// Appends a actor to the list
/// \param entity: Entity to add
/// \param owner: Line to which the entity should be appended as child
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeRow ActorList::append(const HEntity& entity, Gtk::TreeRow& owner) {
    TRACE7("ActorList::append (const HEntity&, Gtk::TreeRow&)");

    Gtk::TreeRow newLine(*mOwnerObjects->append(owner.children()));
    newLine[colActors.entry] = entity;
    update(newLine);
    return newLine;
}

//-----------------------------------------------------------------------------
/// Sets the values of the line to the values of the stored entity
/// \param row: Row to update
//-----------------------------------------------------------------------------
void ActorList::update(Gtk::TreeRow& row) {
    HEntity obj(row[colActors.entry]);
    if (HFilm film(std::dynamic_pointer_cast<Film>(obj)); film) {
        row[colActors.name] = film->getName();
        row[colActors.year] = film->getYear().toString();

        unsigned int g(film->getGenre());
        if (g >= genres.size())
            g = 0;
        row[colActors.genre] = genres.getGenre(g);
        row[colActors.editable] = false;
    }
    else {
        HActor actor(std::dynamic_pointer_cast<Actor>(obj));
        contract_assert(actor);
        row[colActors.name] = actor->getName();
        row[colActors.year] = actor->getLifespan();
        row[colActors.editable] = true;
    }
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void ActorList::valueChanged(const Glib::ustring& path, const Glib::ustring& value, unsigned int column) {
    TRACE9("ActorList::valueChanged (2x const Glib::ustring&, unsigned int) - " << path << "->" << value);

    Gtk::TreeModel::iterator iRow(mOwnerObjects->get_iter(Gtk::TreeModel::Path(path)));
    Gtk::TreeModel::Row row(*iRow);
    Glib::ustring oldValue;

    try {
        HEntity hEntity(row[colActors.entry]);
        HActor actor(std::dynamic_pointer_cast<Actor>(hEntity));
        contract_assert(actor);

        switch (column) {
        case 0:
            if (!value.empty()) {
                Gtk::TreeModel::const_iterator i(findName(value));
                if ((i != iRow) && (i != mOwnerObjects->children().end()))
                    throw YGP::InvalidValue(Glib::ustring::compose(_("Entry `%1' already exists!"), value));
            }
            actor->setName(value);
            oldValue = row[colActors.name];
            row[colActors.name] = value;
            break;

        case 1:
            actor->setLifespan(value);
            oldValue = row[colActors.year];
            row[colActors.year] = value;
            break;
        } // end-switch

        if (value != oldValue)
            signalActorChanged.emit(iRow, column, oldValue);
    } // end-try
    catch (const std::exception& e) {
        YGP::StatusObject obj(YGP::StatusObject::ERROR, e.what());
        obj.generalize(_("Invalid value!"));

        XGP::MessageDlg* dlg(XGP::MessageDlg::create(obj));
        dlg->set_title(PACKAGE);
        if (auto* win(dynamic_cast<Gtk::Window*>(get_root())); win)
            dlg->set_transient_for(*win);
    }
}

//-----------------------------------------------------------------------------
/// Selects the passed row (as only one) and centers it
/// \param i: Iterator to row to select
//-----------------------------------------------------------------------------
void ActorList::selectRow(const Gtk::TreeModel::const_iterator& i) {
    Glib::RefPtr<Gtk::TreeSelection> sel(get_selection());
    Gtk::TreePath path(mOwnerObjects->get_path(i));
    scroll_to_row(path, 0.5);
    set_cursor(path);
    sel->select(path);
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the line having the name stored. Only editable lines
/// are considered
/// \param name: Name of object to find
/// \param level: Level of recursion (0: None)
/// \param begin: Start object
/// \param end: End object
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or mOwnerObjects->children ().end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator ActorList::findName(const Glib::ustring& name, unsigned int level, Gtk::TreeModel::iterator begin,
                                             Gtk::TreeModel::iterator end) const {
    while (begin != end) {
        if (begin->get_value(colActors.editable) && (name == begin->get_value(colActors.name)))
            return begin;

        if (level && !begin->children().empty()) {
            const Gtk::TreeModel::iterator res(findName(name, level - 1, begin->children().begin(), begin->children().end()));
            if (res != mOwnerObjects->children().end())
                return res;
        }
        ++begin;
    } // end-while
    return mOwnerObjects->children().end();
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the line having entry stored
/// \param entry: Entry to find
/// \param level: Level of recursion (0: None)
/// \param begin: Start object
/// \param end: End object
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or mOwnerObjects->children ().end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator ActorList::findEntity(const HEntity& entry, unsigned int level, Gtk::TreeModel::iterator begin,
                                               Gtk::TreeModel::iterator end) const {
    while (begin != end) {
        if (entry == begin->get_value(colActors.entry))
            return begin;

        if (level && !begin->children().empty()) {
            const Gtk::TreeModel::iterator res(findEntity(entry, level - 1, begin->children().begin(), begin->children().end()));
            if (res != mOwnerObjects->children().end())
                return res;
        }
        ++begin;
    } // end-while
    return mOwnerObjects->children().end();
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring first names,
/// articles, ...)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortByName(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    Glib::ustring nameA(a->get_value(colActors.name));
    Glib::ustring nameB(b->get_value(colActors.name));

    const HEntity entity(getEntityAt(a));
    int rc((typeid(*entity) == typeid(Actor)) ? Actor::removeIgnored(nameA).compare(Actor::removeIgnored(nameB))
                                                    : Film::removeIgnored(nameA).compare(Film::removeIgnored(nameB)));
    if (!rc)
        rc = nameA.compare(nameB);
    return rc;
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the year
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int ActorList::sortByYear(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    YGP::AYear ya;
    YGP::AYear yb;

    HEntity entity(getEntityAt(a));
    if (typeid(*entity) == typeid(Actor)) {
        ya = std::dynamic_pointer_cast<Actor>(entity)->getBorn();
        entity = getEntityAt(b);
        yb = std::dynamic_pointer_cast<Actor>(entity)->getBorn();
    }
    else {
        ya = std::dynamic_pointer_cast<Film>(entity)->getYear();
        entity = getEntityAt(b);
        yb = std::dynamic_pointer_cast<Film>(entity)->getYear();
    }
    return ya.compare(yb);
}
