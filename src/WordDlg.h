#ifndef WORDDLG_H
#define WORDDLG_H

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

#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

namespace Gtk {
class Entry;
class Button;
} // namespace Gtk

/**Table permitting to manipulate the words.
 */
class WordDialog : public Gtk::Grid {
  public:
    static Gtk::Widget* makeDialog() { return new WordDialog(); }
    static void commitDialogData(Gtk::Widget* dialog) pre(dialog);

  protected:
    WordDialog();
    ~WordDialog() override = default;

    void commit();

    class WordColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        WordColumns() { add(word); }

        Gtk::TreeModelColumn<Glib::ustring> word;
    };

    void appendWord(const char* value);
    void appendArticle(const char* value);
    Gtk::TreeModel::Row append(Glib::RefPtr<Gtk::ListStore>& list, const Glib::ustring& value);
    void entryChanged(unsigned int which) pre(which < 2);
    void entrySelected(unsigned int which) pre(which < 2);
    void onAdd(unsigned int which) pre(which < 2);
    void onDelete(unsigned int which) pre(which < 2);

  private:
    WordColumns colWords;
    Glib::RefPtr<Gtk::ListStore> names;
    Glib::RefPtr<Gtk::ListStore> articles;

    Gtk::Entry& txtName;
    Gtk::Entry& txtArticle;

    Gtk::Button& addName;
    Gtk::Button& deleteName;
    Gtk::Button& addArticle;
    Gtk::Button& deleteArticle;

    Gtk::TreeView& lstNames;
    Gtk::TreeView& lstArticles;
};

#endif
