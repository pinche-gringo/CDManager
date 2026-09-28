// PROJECT     : CDManager
// SUBSYSTEM   : CDManager
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 31.10.2004
// COPYRIGHT   : Copyright (C) 2004 - 2019, 2026

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
#include <string>
#include <string_view>

#include <glibmm/main.h>

#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/window.h>

#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XValue.h>

#include "CDType.h"
#include "Genres.h"
#include "LangDlg.h"

#include "FilmList.h"

namespace {

/// Splits a comma-separated list of languages into its (non-empty) parts
/// (skipping empty parts like the formerly used boost::tokenizer)
/// \param values Comma-separated list of languages
/// \returns Lazy view of the non-empty languages
auto splitLanguages(std::string_view values) {
    return values | std::views::split(',') | std::views::filter([](auto&& part) { return !std::ranges::empty(part); }) |
           std::views::transform([](auto&& part) { return std::string(std::from_range, part); });
}

} // namespace

//-----------------------------------------------------------------------------
/// Default constructor
/// \param genres: Genres which should be displayed in the 3rd column
//-----------------------------------------------------------------------------
FilmList::FilmList(const Genres& genres) : OwnerObjectList(genres), mTypes(Gtk::ListStore::create(colTypes)) {
    TRACE9("FilmList::FilmList (const Genres&)");
    mOwnerObjects = Gtk::TreeStore::create(colFilms);
    init(colFilms);

    // Add column "Type"
    auto* renderer(Gtk::make_managed<Gtk::CellRendererCombo>());
    renderer->property_text_column() = 0;
    renderer->property_model() = mTypes;
    renderer->property_editable() = true;
    Gtk::TreeViewColumn* column(Gtk::make_managed<Gtk::TreeViewColumn>(_("Type"), *renderer));
    append_column(*column);
    column->add_attribute(renderer->property_text(), colFilms.type);
    column->add_attribute(renderer->property_visible(), colFilms.chgAll);
    column->set_resizable();

    renderer->signal_edited().connect(
        [this](const Glib::ustring& path, const Glib::ustring& value) { valueChanged(path, value, 0); });

    for (const auto& [_, type] : std::as_const(CDType::getInstance())) {
        Gtk::TreeModel::Row newType(*mTypes->append());
        newType[colTypes.type] = type;
    }

    // Add column "Languages"
    column = Gtk::make_managed<Gtk::TreeViewColumn>(_("Language(s)"));
    column->pack_start(colFilms.lang1, false);
    column->pack_start(colFilms.lang2, false);
    column->pack_start(colFilms.lang3, false);
    column->pack_start(colFilms.lang4, false);
    column->pack_start(colFilms.lang5, false);

    append_column(*column);
    column->set_resizable();
    column->add_attribute(column->get_first_cell()->property_visible(), colFilms.chgAll);

    // Add column "Subtitles"
    column = Gtk::make_managed<Gtk::TreeViewColumn>(_("Subtitles(s)"));
    column->pack_start(colFilms.sub1, false);
    column->pack_start(colFilms.sub2, false);
    column->pack_start(colFilms.sub3, false);
    column->pack_start(colFilms.sub4, false);
    column->pack_start(colFilms.sub5, false);
    column->pack_start(colFilms.sub6, false);
    column->pack_start(colFilms.sub7, false);
    column->pack_start(colFilms.sub8, false);
    column->pack_start(colFilms.sub9, false);
    column->pack_start(colFilms.sub10, false);

    append_column(*column);
    column->set_resizable();
    column->add_attribute(column->get_first_cell()->property_visible(), colFilms.chgAll);

    // Check clicks (before the list handles them) to edit languages/subtitles
    Glib::RefPtr<Gtk::GestureClick> click(Gtk::GestureClick::create());
    click->set_button(GDK_BUTTON_PRIMARY);
    click->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    click->signal_pressed().connect(sigc::mem_fun(*this, &FilmList::onButtonPressed));
    add_controller(click);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
FilmList::~FilmList() { TRACE9("FilmList::~FilmList ()"); }

//-----------------------------------------------------------------------------
/// Appends a film to the list
/// \param film: Film to add
/// \param director: Director of the film
/// \returns Gtk::TreeModel::Row: Inserted row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row FilmList::append(HFilm& film, Gtk::TreeModel::Row& director) {
    TRACE3("FilmList::append (HFilm&, Gtk::TreeModel::Row) - " << (film ? film->getName().c_str() : "None"));

    HEntity obj(film);
    Gtk::TreeModel::Row newFilm(OwnerObjectList::append(obj, director));
    update(newFilm);
    return newFilm;
}

//-----------------------------------------------------------------------------
/// Returns the handle (casted to a HFilm) at the passed position
/// \param row: Row in the list
/// \returns HFilm: Handle of the selected line
//-----------------------------------------------------------------------------
HFilm FilmList::getFilmAt(const Gtk::TreeModel::ConstRow& row) const {
    HFilm film(std::dynamic_pointer_cast<Film>(getObjectAt(row)));
    contract_assert(film);
    TRACE7("CDManager::getFilmAt (const Gtk::TreeModel::ConstRow&) - Selected film: " << film->getId() << '/' << film->getName());
    return film;
}

//-----------------------------------------------------------------------------
/// Sets the name of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void FilmList::setName(HEntity& object, const Glib::ustring& value) {
    (std::dynamic_pointer_cast<Film>(object))->setName(value);
}

//-----------------------------------------------------------------------------
/// Sets the year of the object
/// \param object: Object to change
/// \param value: Value to set
/// \throw std::exception: In case of an error
//-----------------------------------------------------------------------------
void FilmList::setYear(HEntity& object, const Glib::ustring& value) {
    (std::dynamic_pointer_cast<Film>(object))->setYear(value);
}

//-----------------------------------------------------------------------------
/// Sets the genre of the object
/// \param object: Object to change
/// \param value: Value to set
//-----------------------------------------------------------------------------
void FilmList::setGenre(HEntity& object, unsigned int value) { (std::dynamic_pointer_cast<Film>(object))->setGenre(value); }

//-----------------------------------------------------------------------------
/// Returns the name of the first column
/// \returns Glib::ustring: The name of the first colum
//-----------------------------------------------------------------------------
Glib::ustring FilmList::getColumnName() const { return _("Director/Film"); }

//-----------------------------------------------------------------------------
/// Sorts the entries in the listbox according to the name (ignoring articles)
/// \param a: First entry to compare
/// \param a: Second entry to compare
/// \returns int: Value as strcmp
//-----------------------------------------------------------------------------
int FilmList::sortEntity(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const {
    HFilm ha(getFilmAt(a));
    HFilm hb(getFilmAt(b));
    int rc(Film::removeIgnored(ha->getName()).compare(Film::removeIgnored(hb->getName())));
    return rc ? rc : ha->getName().compare(hb->getName());
}

//-----------------------------------------------------------------------------
/// Callback after changing a value in the listbox
/// \param path: Path to changed line
/// \param value: New value of entry
/// \param column: Changed column
//-----------------------------------------------------------------------------
void FilmList::valueChanged(const Glib::ustring& path, const Glib::ustring& value, unsigned int column) {
    TRACE7("FilmList::valueChanged (2x const Glib::ustring&, unsigned int) - " << path << "->" << value);

    Gtk::TreeModel::iterator iRow(mOwnerObjects->get_iter(Gtk::TreeModel::Path(path)));
    Gtk::TreeModel::Row row(*iRow);

    try {
        if (row.parent()) {
            HFilm film(getFilmAt(row));
            Glib::ustring oldValue;
            switch (column) {
            case 0: {
                CDType& type(CDType::getInstance());
                if (!type.exists(value)) {
                    Glib::ustring e(_("Unknown type of media!"));
                    throw(YGP::InvalidValue(e));
                }
                film->setType(value);
                oldValue = row[colFilms.type];
                row[colFilms.type] = value;
                break;
            }

            case 1:
                oldValue = film->getLanguage();
                setLanguage(row, value);
                film->setLanguage(value);
                break;

            case 2:
                oldValue = film->getTitles();
                setTitles(row, value);
                film->setTitles(value);
                break;

            default:
                contract_assert(false);
            } // end-switch

            if (value != oldValue)
                signalObjectChanged.emit(iRow, column + 3, oldValue);
        } // endif object edited
    } // end-try
    catch (std::exception& e) {
        YGP::StatusObject obj(YGP::StatusObject::ERROR, e.what());
        obj.generalize(_("Invalid value!"));

        XGP::MessageDlg* dlg(XGP::MessageDlg::create(obj));
        dlg->set_title(PACKAGE);
        if (auto* win(dynamic_cast<Gtk::Window*>(get_root())); win)
            dlg->set_transient_for(*win);
    }
}

//-----------------------------------------------------------------------------
/// Callback for mouse-clicks in the listbox (called before the list handles
/// the click): Clicking in the language- or subtitle-column of the already
/// selected film opens a dialog to edit those values.
/// \param nPress: Number of presses
/// \param x: X-position of the click (widget-coordinates)
/// \param y: Y-position of the click (widget-coordinates)
//-----------------------------------------------------------------------------
void FilmList::onButtonPressed(int, double x, double y) {
    TRACE9("FilmList::onButtonPressed (int, 2x double) - " << x << '/' << y);

    Gtk::TreeModel::iterator sel(get_selection()->get_selected());
    if (!sel || !sel->parent())
        return;

    int bx, by, cellX, cellY;
    convert_widget_to_bin_window_coords(static_cast<int>(x), static_cast<int>(y), bx, by);

    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column(nullptr);
    if (!get_path_at_pos(bx, by, path, column, cellX, cellY) || (path != mOwnerObjects->get_path(sel)))
        return;

    contract_assert(get_column(4));
    contract_assert(get_column(5));
    if ((column == get_column(4)) || (column == get_column(5))) {
        // Show the dialog after the click has been processed
        Glib::signal_idle().connect_once([this, strPath = path.to_string(), subtitles = (column == get_column(5))]() {
            editLanguages(strPath, subtitles);
        });
    }
}

//-----------------------------------------------------------------------------
/// Shows a dialog to edit the languages or subtitles of a film
/// \param path: Path to the row of the film
/// \param subtitles: Flag, if subtitles (or languages) should be edited
//-----------------------------------------------------------------------------
void FilmList::editLanguages(const Glib::ustring& path, bool subtitles) {
    TRACE9("FilmList::editLanguages (const Glib::ustring&, bool) - " << path << '/' << subtitles);

    Gtk::TreeModel::iterator iRow(mOwnerObjects->get_iter(path));
    if (!iRow)
        return;

    HFilm film(getFilmAt(iRow));
    contract_assert(film);
    std::string values(subtitles ? film->getTitles() : film->getLanguage());
    LanguageDialog dlg(values, subtitles ? 10 : 5, !subtitles);
    if (subtitles)
        dlg.set_title(_("Select subtitles"));

    if (auto* win(dynamic_cast<Gtk::Window*>(get_root())); win)
        dlg.set_transient_for(*win);
    XGP::runModal(dlg);

    if (values != (subtitles ? film->getTitles() : film->getLanguage()))
        valueChanged(path, values, subtitles ? 2 : 1);
}

//-----------------------------------------------------------------------------
/// Sets the language-flags according to the passed value
/// \param row: Row to set the languages for
/// \param languages: Languages to show
//-----------------------------------------------------------------------------
void FilmList::setLanguage(Gtk::TreeModel::Row& row, const std::string& languages) {
    auto langs(splitLanguages(languages));
    auto l(langs.begin());
    bool countSet(false);
    const std::array columns{&colFilms.lang1, &colFilms.lang2, &colFilms.lang3, &colFilms.lang4, &colFilms.lang5};

    for (unsigned int i(0); i < columns.size(); ++i)
        if (l != langs.end()) {
            row[*columns[i]] = Language::findFlag(*l);
            ++l;
        }
        else {
            row[*columns[i]] = Glib::RefPtr<Gdk::Pixbuf>();
            if (!countSet) {
                row[colFilms.langs] = i;
                countSet = true;
            }
        }
}

//-----------------------------------------------------------------------------
/// Sets the subtitle-flags according to the passed value
/// \param row: Row to set the subtitles for
/// \param titles: Subtitles to show
//-----------------------------------------------------------------------------
void FilmList::setTitles(Gtk::TreeModel::Row& row, const std::string& titles) {
    auto langs(splitLanguages(titles));
    auto l(langs.begin());
    bool countSet(false);
    const std::array columns{&colFilms.sub1, &colFilms.sub2, &colFilms.sub3, &colFilms.sub4, &colFilms.sub5,
                             &colFilms.sub6, &colFilms.sub7, &colFilms.sub8, &colFilms.sub9, &colFilms.sub10};

    for (unsigned int i(0); i < columns.size(); ++i)
        if (l != langs.end()) {
            row[*columns[i]] = Language::findFlag(*l);
            ++l;
        }
        else {
            TRACE1("Set Title: " << titles << " = " << i);
            row[*columns[i]] = Glib::RefPtr<Gdk::Pixbuf>();
            if (!countSet) {
                row[colFilms.titles] = i;
                countSet = true;
            }
        }
}

//-----------------------------------------------------------------------------
/// Updates the displayed films, showing them in the passed language
/// \param lang: Language, in which the films should be displayed
//-----------------------------------------------------------------------------
void FilmList::update(const std::string& lang) {
    TRACE9("FilmList::update (const std::string&) - " << lang);

    for (auto& director : mOwnerObjects->children()) {
        for (auto& row : director.children()) {
            HFilm film(getFilmAt(row));
            contract_assert(film);
            TRACE9("FilmList::update (const std::string&) - Updating " << film->getName(lang));
            row[colOwnerObjects->name] = film->getName(lang);
        }
    }
}

//-----------------------------------------------------------------------------
/// Updates the displayed films; actualises the displayed values with the
/// values stored in the object in the entity-column
/// \param row: Row to update
//-----------------------------------------------------------------------------
void FilmList::update(Gtk::TreeModel::Row& row) {
    if (row.parent()) {
        HFilm film(getFilmAt(row));
        row[colFilms.name] = film->getName();
        row[colFilms.year] = film->getYear().toString();
        changeGenre(row, film->getGenre());

        row[colFilms.type] = CDType::getInstance()[film->getType()];

        setLanguage(row, film->getLanguage());
        setTitles(row, film->getTitles());
    }
    OwnerObjectList::update(row);
}
