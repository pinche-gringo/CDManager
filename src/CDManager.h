#ifndef CDMANAGER_H
#define CDMANAGER_H

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

#if !defined(WITH_ACTORS) || !defined(WITH_RECORDS) || !defined(WITH_FILMS)
#    error Need WITH_ACTORS, WITH_RECORDS and WITH_FILMS defined
#endif

#include <array>
#include <map>
#include <vector>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <gtkmm/notebook.h>
#include <gtkmm/shortcutcontroller.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/treeview.h>

#include "Genres.h"

#include <YGP/Relation.h>

#include <XGP/XApplication.h>

// Forward declarations
class NBPage;
class Options;
namespace YGP {
class Entity;
class StatusObject;
} // namespace YGP

/**Class for application to manage CDs (audio and video)
 */
class CDManager : public XGP::XApplication {
  public:
    // Manager functions
    explicit CDManager(Options& options);
    ~CDManager() override;

    CDManager(const CDManager&) = delete;
    const CDManager& operator=(const CDManager&) = delete;

  private:
    // Event-handling
    void save();
    void showLogin();
    void logout();
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
    void export2HTML();
#endif

    void showStatistics();
    void editPreferences();
    void savePreferences();

    void showAboutbox() override;
    const char* getHelpfile() override;
    void pageSwitched(Gtk::Widget* page, guint iPage) pre(iPage < 3);
    void enablePageMenus(bool enable);

    bool login(const Glib::ustring& user, const Glib::ustring& pwd);
    void loadDatabase();

    void enableMenus(bool enable);
    void exit();
    void querySave();
    bool on_close_request() override;
    void showError(const Glib::ustring& msg, const Glib::ustring& title = Glib::ustring());
    Glib::RefPtr<Gio::SimpleAction> addMenuEntry(const Glib::RefPtr<Gio::Menu>& menu, const Glib::ustring& label,
                                                 const char* action, const sigc::slot<void()>& callback,
                                                 const Glib::ustring& accel = Glib::ustring());

    static constexpr unsigned int WIDTH{800};
    static constexpr unsigned int HEIGHT{600};

    static constexpr const char* DBNAME{"CDMedia"};

    Genres recGenres;
    Genres filmGenres;

    Gtk::Notebook nb;
    Gtk::Statusbar status;

    enum {
        LOGIN = 0,
        SAVE,
        LOGOUT,
        STATISTICS,
        SAVE_PREFS,
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
        EXPORT,
#endif
        LAST
    };
    std::array<Glib::RefPtr<Gio::SimpleAction>, LAST> apMenus;

    Glib::RefPtr<Gio::Menu> menuEdit;               ///< Edit-menu; filled by the pages
    Glib::RefPtr<Gio::Menu> menuOther;              ///< Additional top-level menus of the pages
    Glib::RefPtr<Gio::SimpleActionGroup> grpPage;   ///< Actions ("page.*") of the current page
    Glib::RefPtr<Gtk::ShortcutController> ctrlPage; ///< Shortcuts of the current page
    Glib::RefPtr<Gtk::ShortcutController> ctrlMain; ///< Shortcuts of the main menu
    bool pageMenusOn{false};                        ///< Flag, if the page-menus are enabled
    Options& opt;

    std::array<NBPage*, WITH_ACTORS + WITH_FILMS + WITH_RECORDS> pages{};
};

#endif
