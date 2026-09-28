// PROJECT     : CDManager
// SUBSYSTEM   : WordDlg
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 20.4.2005
// COPYRIGHT   : Copyright (C) 2005, 2006, 2010, 2011, 2026

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
#include <stdexcept>
#include <ranges>
#include <tuple>

#include <glibmm/ustring.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Trace.h>

#include <XGP/XDialog.h>

#include "Words.h"

#include "WordDlg.h"

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
WordDialog::WordDialog()
    : names(Gtk::ListStore::create(colWords)), articles(Gtk::ListStore::create(colWords)),
      txtName(*Gtk::make_managed<Gtk::Entry>()), txtArticle(*Gtk::make_managed<Gtk::Entry>()),
      addName(*Gtk::make_managed<Gtk::Button>(_("_Add"), true)), deleteName(*Gtk::make_managed<Gtk::Button>(_("_Delete"), true)),
      addArticle(*Gtk::make_managed<Gtk::Button>(_("A_dd"), true)),
      deleteArticle(*Gtk::make_managed<Gtk::Button>(_("D_elete"), true)), lstNames(*Gtk::make_managed<Gtk::TreeView>(names)),
      lstArticles(*Gtk::make_managed<Gtk::TreeView>(articles)) {
    TRACE9("WordDialog::WordDialog ()");
    Gtk::ScrolledWindow& scrlNames(*Gtk::make_managed<Gtk::ScrolledWindow>());
    Gtk::ScrolledWindow& scrlArticles(*Gtk::make_managed<Gtk::ScrolledWindow>());

    Gtk::Box& bboxNames(*Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 3));
    Gtk::Box& bboxArticle(*Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 3));
    bboxNames.set_halign(Gtk::Align::END);
    bboxArticle.set_halign(Gtk::Align::END);

    lstNames.append_column(_("First names"), colWords.word);
    lstArticles.append_column(_("Articles"), colWords.word);

    for (const auto [i, view] : std::views::enumerate(std::array{&lstNames, &lstArticles})) {
        view->set_headers_visible(true);
        view->set_enable_search(true);
        view->set_search_column(colWords.word);

        Glib::RefPtr<Gtk::TreeSelection> listSelection(view->get_selection());
        listSelection->set_mode(Gtk::SelectionMode::MULTIPLE);
        listSelection->signal_changed().connect(
            sigc::bind(sigc::mem_fun(*this, &WordDialog::entrySelected), static_cast<unsigned int>(i)));
    }
    scrlNames.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrlNames.set_has_frame(true);
    scrlNames.set_expand(true);
    scrlNames.set_child(lstNames);

    scrlArticles.set_has_frame(true);
    scrlArticles.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrlArticles.set_hexpand(true);
    scrlArticles.set_child(lstArticles);

    txtName.set_max_length(32);
    txtName.set_activates_default(true);
    txtArticle.set_max_length(9);
    txtArticle.set_activates_default(true);
    txtName.set_hexpand(true);
    txtArticle.set_hexpand(true);

    addName.set_sensitive(false);
    deleteName.set_sensitive(false);
    bboxNames.append(addName);
    bboxNames.append(deleteName);

    addArticle.set_sensitive(false);
    deleteArticle.set_sensitive(false);
    bboxArticle.append(addArticle);
    bboxArticle.append(deleteArticle);

    set_row_spacing(3);
    set_column_spacing(5);
    set_margin(5);
    attach(scrlNames, 0, 0);
    attach(scrlArticles, 1, 0);
    attach(txtName, 0, 1);
    attach(txtArticle, 1, 1);
    attach(bboxNames, 0, 2);
    attach(bboxArticle, 1, 2);

    if (Words::areAvailable()) {
        addName.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::onAdd), 0));
        deleteName.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::onDelete), 0));
        addArticle.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::onAdd), 1));
        deleteArticle.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::onDelete), 1));

        txtName.signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::entryChanged), 0));
        txtArticle.signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &WordDialog::entryChanged), 1));

        // Fill listboxes
        TRACE1("WordDialog::WordDialog () - Words: " << Words::cNames() << "; Articles: " << Words::cArticles());
        Words::forEachName(0U, Words::cNames(), *this, &WordDialog::appendWord);
        Words::forEachArticle(0, Words::cArticles(), *this, &WordDialog::appendArticle);

        names->set_sort_column(colWords.word, Gtk::SortType::ASCENDING);
        articles->set_sort_column(colWords.word, Gtk::SortType::ASCENDING);
    }
    else
        set_sensitive(false);
}

//-----------------------------------------------------------------------------
/// Callback after an entryfield has been  changed; the buttons are enabled
/// accordingly.
/// \param which: Flag, which field has been changed
//-----------------------------------------------------------------------------
void WordDialog::entryChanged(unsigned int which) {
    TRACE8("WordDialog::entryChanged (unsigned int) - " << which);
    Words::values* shMem(Words::getInfo());
    contract_assert(shMem);

    const std::array<Gtk::Entry*, 2> fields{&txtName, &txtArticle};
    const std::array<Gtk::Button*, 2> buttons{&addName, &addArticle};

    const std::array<unsigned int, 2> starts{0, shMem->maxEntries - shMem->cArticles};
    const std::array<unsigned int, 2> ends{Words::cNames(), shMem->maxEntries};

    const bool unique(fields[which]->get_text_length()
                      && !Words::containsWord(starts[which], ends[which], fields[which]->get_text()));
    buttons[which]->set_sensitive(unique);
}

//-----------------------------------------------------------------------------
/// Callback after the selection in a list has been changed; the
/// buttons are enabled accordingly.
/// \param which: Flag, which list has been changed
//-----------------------------------------------------------------------------
void WordDialog::entrySelected(unsigned int which) {
    TRACE8("WordDialog::entrySelected (unsigned int) - " << which);
    const std::array<Gtk::TreeView*, 2> lists{&lstNames, &lstArticles};
    const std::array<Gtk::Button*, 2> buttons{&deleteName, &deleteArticle};

    buttons[which]->set_sensitive(!lists[which]->get_selection()->get_selected_rows().empty());
}

//-----------------------------------------------------------------------------
/// Appends a line to the passed model
/// \param model: Model to append line to
/// \param value: Value to append
/// \returns Gtk::TreeModel::Row: Appended row
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row WordDialog::append(Glib::RefPtr<Gtk::ListStore>& list, const Glib::ustring& value) {
    TRACE8("WordDialog::append (Glib::RefPtr<Gtk::ListStore>&, const Glib::ustring&) - " << value);
    Gtk::TreeModel::Row row(*list->append());
    row[colWords.word] = value;
    return row;
}

//-----------------------------------------------------------------------------
/// Appends a line to the names
/// \param value: Value to append
//-----------------------------------------------------------------------------
void WordDialog::appendWord(const char* value) {
    TRACE8("WordDialog::appendWord (const char*) - " << value);
    append(names, value);
}

//-----------------------------------------------------------------------------
/// Appends a line to the articles
/// \param value: Value to append
//-----------------------------------------------------------------------------
void WordDialog::appendArticle(const char* value) {
    TRACE1("WordDialog::appendArticle (const char*) - " << value);
    append(articles, value);
}

//-----------------------------------------------------------------------------
/// Callback after adding a name
/// \param which: Flag, to which list a value should be added
//-----------------------------------------------------------------------------
void WordDialog::onAdd(unsigned int which) {
    const std::array<Gtk::Entry*, 2> fields{&txtName, &txtArticle};
    const std::array<Gtk::Button*, 2> buttons{&addName, &addArticle};
    const std::array<Gtk::TreeView*, 2> lists{&lstNames, &lstArticles};
    std::array models{names, articles};

    contract_assert(fields[which]->get_text_length());
    buttons[which]->set_sensitive(false);
    Gtk::TreeModel::Row row(append(models[which], fields[which]->get_text()));
    lists[which]->scroll_to_row(models[which]->get_path(row.get_iter()), 0.8);
    Glib::RefPtr<Gtk::TreeSelection> sel(lists[which]->get_selection());
    sel->unselect_all();
    sel->select(row.get_iter());

    fields[which]->set_text("");
}

//-----------------------------------------------------------------------------
/// Callback after deleting a name
/// \param which: Flag, from which list a value/values should be removed
//-----------------------------------------------------------------------------
void WordDialog::onDelete(unsigned int which) {
    TRACE8("WordDialog::onDelete (unsigned int) - " << which);
    const std::array<Gtk::TreeView*, 2> lists{&lstNames, &lstArticles};
    const std::array models{names, articles};

    Glib::RefPtr<Gtk::TreeSelection> selection(lists[which]->get_selection());
    for (std::vector<Gtk::TreeModel::Path> list(selection->get_selected_rows()); !list.empty();
         list = selection->get_selected_rows()) {
        Gtk::TreeModel::iterator iter(models[which]->get_iter(list.front()));
        contract_assert(iter);
        models[which]->erase(iter);
    }
}

//-----------------------------------------------------------------------------
/// Commits the changes within the lists
//-----------------------------------------------------------------------------
void WordDialog::commit() {
    TRACE9("WordDialog::commit ()");

    if (Words::areAvailable()) {
        using FnInsert = void (*)(const Glib::ustring& word, unsigned int pos);
        const std::array models{names, articles};
        const std::array<FnInsert, 2> fnInsert{&Words::addName2Ignore, &Words::addArticle};
        static_assert(std::tuple_size_v<decltype(models)> == std::tuple_size_v<decltype(fnInsert)>);

        Words::values* shMem(Words::getInfo());
        contract_assert(shMem);
        shMem->cArticles = shMem->cNames = 0;
        shMem->used = 0;

        try {
            for (const auto& [model, insert] : std::views::zip(models, fnInsert))
                for (const auto& row : model->children())
                    insert(row.get_value(colWords.word), Words::POS_END);
        }
        catch (const std::length_error& err) { // The storage for the words is full
            Gtk::MessageDialog dlg(err.what(), false, Gtk::MessageType::ERROR);
            XGP::runModal(dlg);
        }
    }
}

//-----------------------------------------------------------------------------
/// Commits the changes within the lists
/// \param dialog
//-----------------------------------------------------------------------------
void WordDialog::commitDialogData(Gtk::Widget* dialog) {
    dynamic_cast<WordDialog*>(dialog)->commit();
}
