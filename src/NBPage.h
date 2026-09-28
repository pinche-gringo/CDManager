#ifndef NBPAGE_H
#define NBPAGE_H

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

#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glibmm/ustring.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <gtkmm/shortcutcontroller.h>
#include <gtkmm/treepath.h>

#include <YGP/Entity.h>

#include <sigc++/trackable.h>

using HEntity = std::shared_ptr<YGP::Entity>;

// Forward declarations
namespace Gtk {
class Widget;
class Statusbar;
} // namespace Gtk

/**Baseclass for notebook-pages
 */
class NBPage : public sigc::trackable {
  public:
    virtual ~NBPage() = default;

    virtual Gtk::Widget* getWindow() const { return widget; }

    [[nodiscard]] bool isLoaded() const { return loaded; }

    virtual void loadData() = 0;
    virtual void saveData() = 0;
    virtual void getFocus() = 0;
    /// Adds the page-related menu-entries to menuEdit (the Edit-menu) and
    /// optional top-level menus to menuOther. The according actions are
    /// added to grpAction (which is inserted as "page" into the window), the
    /// keyboard shortcuts to shortcuts.
    virtual void addMenu(Glib::RefPtr<Gio::Menu> menuEdit, Glib::RefPtr<Gio::Menu> menuOther,
                         Glib::RefPtr<Gio::SimpleActionGroup> grpAction, Glib::RefPtr<Gtk::ShortcutController> shortcuts) = 0;
    virtual void removeMenu();
    virtual void deleteSelection() = 0;
    virtual void undo() = 0;
    virtual void clear();
    virtual void export2HTML(unsigned int fd, const std::string& lang);

    [[nodiscard]] bool isChanged() const { return !aUndo.empty(); }

    NBPage(const NBPage&) = delete;
    NBPage& operator=(const NBPage&) = delete;

    static void addMenuEntry(const Glib::RefPtr<Gio::Menu>& menu, const Glib::ustring& label, const Glib::ustring& action,
                             const Glib::ustring& accel = Glib::ustring(),
                             const Glib::RefPtr<Gtk::ShortcutController>& shortcuts = {})
        pre(menu);

  protected:
    NBPage(Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave)
        : menuSave(std::move(menuSave)), statusbar(status) {}

    Gtk::Widget* widget{nullptr};
    Glib::RefPtr<Gio::SimpleAction> menuSave;
    Gtk::Statusbar& statusbar;

    enum SELECTED { NONE_SELECTED, OWNER_SELECTED, OBJECT_SELECTED };
    void enableEdit(SELECTED selected);
    void showStatus(const Glib::ustring& msgStatus);
    void showError(const Glib::ustring& msg, const Glib::ustring& title = Glib::ustring());
    void addStatusWidget(Gtk::Widget& widget);
    void removeStatusWidget(Gtk::Widget& widget);
    void enableSave(bool on = true) { menuSave->set_enabled(on); }

    enum { NEW1, NEW2, NEW3, UNDO, DELETE, LAST };
    Glib::RefPtr<Gio::SimpleAction> apMenus[LAST];

    /**Undo-info
     */
    class Undo {
      public:
        enum CHGSPEC { UNDEFINED = 0, INSERT, DELETE, CHANGED };
        Undo() = default;
        Undo(CHGSPEC chg, unsigned int what, unsigned int col, HEntity entity, const Gtk::TreePath& row,
             const Glib::ustring& value);

        [[nodiscard]] unsigned int how() const { return chgSpec.how; }
        [[nodiscard]] unsigned int what() const { return chgSpec.what; }
        [[nodiscard]] unsigned int column() const { return chgSpec.column; }

        [[nodiscard]] const HEntity getEntity() const { return entity; }
        [[nodiscard]] const Gtk::TreePath& getPath() const { return row; }
        [[nodiscard]] const Glib::ustring& getValue() const { return value; }

      private:
        struct {
            unsigned int column : 16 {0};
            unsigned int how : 2 {UNDEFINED};
            unsigned int what : 3 {0};
        } chgSpec;
        HEntity entity;
        Gtk::TreePath row;
        Glib::ustring value;
    };

    bool loaded{false};
    std::stack<Undo> aUndo;
    std::map<HEntity, HEntity> delRelation;
};

#endif
