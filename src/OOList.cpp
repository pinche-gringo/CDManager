// PROJECT     : CDManager
// SUBSYSTEM   : OwnerObjectList
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 25.11.2004
// COPYRIGHT   : Copyright (C) 2004 - 2007, 2009 - 2011, 2026

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
#include <array>
#include <typeinfo>

#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/window.h>

#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>

#include <XGP/XValue.h>

#include "OOList.h"

//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
OwnerObjectList::OwnerObjectList(const Genres& genres)
    : genres(genres), mGenres(Gtk::ListStore::create(colGenres)) {
    TRACE9("OwnerObjectList::OwnerObjectList (const Genres&)");
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
OwnerObjectList::~OwnerObjectList() { TRACE9("OwnerObjectList::~OwnerObjectList ()"); }

//-----------------------------------------------------------------------------
/// Initializes the class
/// \param cols: Columns of the model
//-----------------------------------------------------------------------------
void OwnerObjectList::init(const OwnerObjectColumns& cols) {
    TRACE9("OwnerObject::init ()");
    updateGenres();

    colOwnerObjects = &cols;
    set_model(mOwnerObjects);

    append_column(getColumnName(), cols.name);
    append_column(_("Year"), cols.year);

    const std::array index{cols.name.index(), cols.year.index()};
    for (unsigned int i(0); i < index.size(); ++i) {
        Gtk::TreeViewColumn* column(get_column(i));
        column->set_sort_column(index[i]);
        column->set_resizable();

        Gtk::CellRenderer* r(get_column_cell_renderer(i));
        contract_assert(r);
        contract_assert(typeid(*r) == typeid(Gtk::CellRendererText));
        Gtk::CellRendererText* rText(dynamic_cast<Gtk::CellRendererText*>(r));
        rText->property_editable() = true;
        rText->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &OwnerObjectList::valueChanged), i));
    }

    Gtk::CellRendererCombo* renderer(Gtk::make_managed<Gtk::CellRendererCombo>());
    renderer->property_text_column() = 0;
    renderer->property_model() = mGenres;
    Gtk::TreeViewColumn* column(Gtk::make_managed<Gtk::TreeViewColumn>(_("Genre"), *renderer));
    append_column(*column);
    column->add_attribute(renderer->property_text(), cols.genre);
    column->add_attribute(renderer->property_editable(), cols.chgAll);

    column->set_sort_column(cols.genre.index());
    column->set_resizable();

    renderer->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &OwnerObjectList::valueChanged), 2));

    mOwnerObjects->set_sort_func(cols.name, sigc::mem_fun(*this, &OwnerObjectList::sortByName));
    mOwnerObjects->set_sort_func(cols.year, sigc::mem_fun(*this, &OwnerObjectList::sortByYear));
    mOwnerObjects->set_sort_func(cols.genre, sigc::mem_fun(*this, &OwnerObjectList::sortByGenre));
    set_headers_clickable();
}

//-----------------------------------------------------------------------------
/// Appends an object to an owner in the list
/// \param object: Object to add
/// \param owner: Owner to add the object to
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::append(HEntity& object, Gtk::TreeModel::Row& owner) {
    TRACE3("OwnerObjectList::append (HEntity&, Gtk::TreeModel::Row&)");
    contract_assert(colOwnerObjects);

    Gtk::TreeModel::Row newObj(*mOwnerObjects->append(owner.children()));
    newObj[colOwnerObjects->entry] = object;
    return newObj;
}

//-----------------------------------------------------------------------------
/// Appends an owner to the list
/// \param owner: Owner to add
/// \param pos: Position in model for insert
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row OwnerObjectList::insert(const HCelebrity& owner, const Gtk::TreeModel::iterator& pos) {
    TRACE3("OwnerObjectList::insert (const HCelebrity&, const Gtk::TreeModel::iterator&) - "
           << (owner ? owner->getName().c_str() : "None"));
    contract_assert(colOwnerObjects);

    Gtk::TreeModel::Row newOwner(*mOwnerObjects->insert(pos));
    set(newOwner, owner);
    return newOwner;
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void OwnerObjectList::valueChanged(const Glib::ustring& path, const Glib::ustring& value, unsigned int column) {
    TRACE9("OwnerObjectList::valueChanged (2x const Glib::ustring&, unsigned int) - " << path << "->" << value);
    contract_assert(colOwnerObjects);

    Gtk::TreeModel::iterator iRow(mOwnerObjects->get_iter(Gtk::TreeModel::Path(path)));
    Gtk::TreeModel::Row row(*iRow);
    Glib::ustring oldValue;

    try {
        if (row.parent()) {
            HEntity object(getObjectAt(row));

            // First check, if value is valid
            switch (column) {
            case 0:
                if (!value.empty()) {
                    Gtk::TreeModel::const_iterator i(getObject(row.parent(), value));
                    if ((i != iRow) && (i != row.parent()->children().end()))
                        throw YGP::InvalidValue(Glib::ustring::compose(_("Entry `%1' already exists!"), value));
                }
                oldValue = row[colOwnerObjects->name];
                row[colOwnerObjects->name] = value;
                setName(object, value);
                break;

            case 1:
                setYear(object, value);
                oldValue = row[colOwnerObjects->year];
                row[colOwnerObjects->year] = value;
                break;

            case 2: {
                oldValue = row[colOwnerObjects->genre];
                oldValue = Glib::ustring(1, static_cast<char>(genres.getId(oldValue)));

                const int g(genres.getId(value));
                if (g == -1)
                    throw YGP::InvalidValue(_("Unknown genre!"));
                setGenre(object, g);
                row[colOwnerObjects->genre] = value;
                break;
            }
            } // endswitch

            if (value != oldValue)
                signalObjectChanged.emit(iRow, column, oldValue);
        } // endif object edited
        else {
            HCelebrity celeb(getCelebrityAt(row));
            contract_assert(celeb);

            switch (column) {
            case 0:
                if (!value.empty()) {
                    // Check if changes are valid
                    Gtk::TreeModel::const_iterator i(getOwner(value));
                    if ((i != iRow) && (i != mOwnerObjects->children().end()))
                        throw YGP::InvalidValue(Glib::ustring::compose(_("Entry `%1' already exists!"), value));
                }
                oldValue = row[colOwnerObjects->name];
                celeb->setName(value);
                row[colOwnerObjects->name] = celeb->getName();
                break;

            case 1:
                celeb->setLifespan(value);
                oldValue = row[colOwnerObjects->year];
                row[colOwnerObjects->year] = celeb->getLifespan();
                break;
            } // end-switch

            if ((value != oldValue) && (column < 2))
                signalOwnerChanged.emit(iRow, column, oldValue);
        } // end-else director edited
    } // end-try
    catch (std::exception& e) {
        YGP::StatusObject obj(YGP::StatusObject::ERROR, e.what());
        obj.generalize(_("Invalid value!"));

        XGP::MessageDlg* dlg(XGP::MessageDlg::create(obj));
        dlg->set_title(PACKAGE);
        Gtk::Window* win(dynamic_cast<Gtk::Window*>(get_root()));
        if (win)
            dlg->set_transient_for(*win);
    }
}

//-----------------------------------------------------------------------------
/// Sets the genres list
//-----------------------------------------------------------------------------
void OwnerObjectList::updateGenres() {
    TRACE9("OwnerObjectList::updateGenres () - Genres: " << genres.size());

    mGenres->clear();
    for (unsigned int i(0); i < genres.size(); ++i) {
        Gtk::TreeModel::Row newGenre(*mGenres->append());
        newGenre[colGenres.genre] = (genres.getGenre(i));
    }
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HEntity) at the passed position
/// \param row: Row in the list
/// \returns HEntity: Handle of the passed line
//-----------------------------------------------------------------------------
HEntity OwnerObjectList::getObjectAt(const Gtk::TreeModel::ConstRow& row) const {
    contract_assert(colOwnerObjects);
    HEntity hEntity(row.get_value(colOwnerObjects->entry));
    contract_assert(hEntity);
    return hEntity;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HCelebrity) at the passed position
/// \param row: Row in the list
/// \returns HCelebrity: Handle of the selected line
//-----------------------------------------------------------------------------
HCelebrity OwnerObjectList::getCelebrityAt(const Gtk::TreeModel::ConstRow& row) const {
    contract_assert(colOwnerObjects);
    TRACE9("GetCelibrity: " << row.get_value(colOwnerObjects->name));
    HCelebrity owner(std::dynamic_pointer_cast<Celebrity>(row.get_value(colOwnerObjects->entry)));
    contract_assert(owner);
    TRACE7("CDManager::getCelebrityAt (const Gtk::TreeModel::ConstRow&) - Selected: " << owner->getId() << '/'
                                                                                      << owner->getName());
    return owner;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setName(HEntity&, const Glib::ustring&) {}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setYear(HEntity&, const Glib::ustring&) {}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
/// \remarks To be implemented
//-----------------------------------------------------------------------------
void OwnerObjectList::setGenre(HEntity&, unsigned int) {}

//-----------------------------------------------------------------------------
/// Sets the genre in the passed row
/// \param row: Row to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void OwnerObjectList::changeGenre(Gtk::TreeModel::Row& row, unsigned int value) {
    TRACE9("OwnerObjectList::changeGenre (Gtk::TreeModel::Row&, unsigned int) - " << value);
    contract_assert(colOwnerObjects);

    if (value >= genres.size())
        value = 0;
    row[colOwnerObjects->genre] = genres.getGenre(value);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring first names,
/// articles, ...)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByName(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    if (a->parent())
        return sortEntity(a, b);
    else
        return sortOwner(a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the year
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByYear(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    if (a->parent()) {
        const YGP::AYear ya(a->get_value(colOwnerObjects->year));
        const YGP::AYear yb(b->get_value(colOwnerObjects->year));
        return ya.compare(yb);
    }
    else
        return sortOwner(a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the genre
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortByGenre(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    if (a->parent()) {
        contract_assert(b->parent());

        const Glib::ustring sa(a->get_value(colOwnerObjects->genre));
        const Glib::ustring sb(b->get_value(colOwnerObjects->genre));
        return sa.compare(sb);
    }
    else
        return sortOwner(a, b);
}

//-----------------------------------------------------------------------------
/// Sorts the owner in the listbox according to the name
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortOwner(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    const HCelebrity ha(getCelebrityAt(a));
    contract_assert(ha);
    const HCelebrity hb(getCelebrityAt(b));
    contract_assert(hb);

    TRACE9("OwnerObjectList::sortOwner (2x const Gtk::TreeModel::const_iterator&) - " << ha->getName() << "<->" << hb->getName());
    const int rc(Celebrity::removeIgnored(ha->getName()).compare(Celebrity::removeIgnored(hb->getName())));
    return rc ? rc : (ha->getName() < hb->getName());
}

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int OwnerObjectList::sortEntity(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    contract_assert(a->parent());
    contract_assert(b->parent());
    contract_assert(colOwnerObjects);

    const Glib::ustring sa(a->get_value(colOwnerObjects->name));
    const Glib::ustring sb(b->get_value(colOwnerObjects->name));
    TRACE9("OwnerObjectList::sortEntity (2x const Gtk::TreeModel::const_iterator&) - " << sa << '/' << sb << '='
                                                                                       << sa.compare(sb));
    return sa.compare(sb);
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the owner having the passed value as name
/// \param name: Name of entry
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getOwner(const Glib::ustring& name) const {
    contract_assert(colOwnerObjects);

    auto owners(mOwnerObjects->children());
    return std::ranges::find(owners, name, [this](const Gtk::TreeModel::Row& row) { return row.get_value(colOwnerObjects->name); });
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the owner identified by the passed handle
/// \param owner: Handle to owner
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getOwner(const HCelebrity& owner) const {
    contract_assert(colOwnerObjects);

    auto owners(mOwnerObjects->children());
    return std::ranges::find_if(owners,
                                [this, &owner](const Gtk::TreeModel::Row& row) { return owner == row.get_value(colOwnerObjects->entry); });
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children having the passed value as name
/// \param parent: Parent row
/// \param name: Name of entry
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject(const Gtk::TreeModel::iterator& parent, const Glib::ustring& name) const {
    contract_assert(colOwnerObjects);

    auto objects(parent->children());
    return std::ranges::find(objects, name, [this](const Gtk::TreeModel::Row& row) { return row.get_value(colOwnerObjects->name); });
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children identified by the passed object
/// \param parent: Parent row
/// \param object: Entry to find
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject(const Gtk::TreeModel::iterator& parent, const HEntity& object) const {
    contract_assert(colOwnerObjects);

    auto objects(parent->children());
    return std::ranges::find(objects, object, [this](const Gtk::TreeModel::Row& row) { return row.get_value(colOwnerObjects->entry); });
}

//-----------------------------------------------------------------------------
/// Returns an iterator to the children identified by the passed object
/// \param object: Entry to find
/// \returns Gtk::TreeModel::iterator: Iterator to found entry or end ().
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator OwnerObjectList::getObject(const HEntity& object) const {
    contract_assert(colOwnerObjects);

    for (auto& owner : mOwnerObjects->children())
        if (auto i(getObject(owner.get_iter(), object)); i != owner.children().end())
            return i;
    return mOwnerObjects->children().end();
}

//-----------------------------------------------------------------------------
/// Selects the passed row (as only one) and centers it
/// \param i: Iterator to row to select
//-----------------------------------------------------------------------------
void OwnerObjectList::selectRow(const Gtk::TreeModel::const_iterator& i) {
    Glib::RefPtr<Gtk::TreeSelection> sel(get_selection());
    Gtk::TreePath path(mOwnerObjects->get_path(i));
    scroll_to_row(path, 0.5);
    set_cursor(path);
    sel->select(path);
}

//-----------------------------------------------------------------------------
/// Sets the entry to display
/// \param row: Row to update
/// \param obj: Object, whose values to set
//-----------------------------------------------------------------------------
void OwnerObjectList::set(Gtk::TreeModel::Row& row, const HEntity& obj) {
    row[colOwnerObjects->entry] = obj;
    update(row);
}

//-----------------------------------------------------------------------------
/// Updates the displayed entry
/// \param row: Row to update
//-----------------------------------------------------------------------------
void OwnerObjectList::update(Gtk::TreeModel::Row& row) {
    if (row.parent())
        row[colOwnerObjects->chgAll] = true;
    else {
        HCelebrity owner(getCelebrityAt(row));
        row[colOwnerObjects->name] = owner->getName();
        row[colOwnerObjects->year] = owner->getLifespan();
        row[colOwnerObjects->chgAll] = false;
    }
}
