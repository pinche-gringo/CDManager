// PROJECT     : CDManager
// SUBSYSTEM   : CDManager
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 10.10.2004
// COPYRIGHT   : Copyright (C) 2004 - 2011, 2026

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

#include <clocale>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include <glibmm/main.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/popovermenubar.h>
#include <gtkmm/scrolledwindow.h>

// TRACELEVEL 1 shows shared-memory key; TRACELEVEL 9 shows password
#include <YGP/Check.h>
#include <YGP/File.h>
#include <YGP/INIFile.h>
#include <YGP/Trace.h>

#include <XGP/LoginDlg.h>
#include <XGP/XAbout.h>

#include "CDAppl.h"
#include "LangDlg.h"
#include "Language.h"
#include "Options.h"
#include "SaveCeleb.h"
#include "Settings.h"
#include "Statistics.h"
#include "Storage.h"
#include "Words.h"

#if WITH_ACTORS == 1
#    include "PActors.h"
#endif
#if WITH_FILMS == 1
#    include "PFilms.h"
#    include <boost/tokenizer.hpp>
#endif
#if WITH_RECORDS == 1
#    include "PRecords.h"
#endif

#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
#    include <YGP/Process.h>
#    include <boost/tokenizer.hpp>
#endif

#include "CDManager.h"

const unsigned int CDManager::WIDTH(800);
const unsigned int CDManager::HEIGHT(600);

const char* const CDManager::DBNAME("CDMedia");

#include "IconAuthor.h"
#include "IconProgram.h"

//-----------------------------------------------------------------------------
/// Defaultconstructor; all widget are created
/// \param options: Options for the program
/// \param user: User for database
/// \param pwd: Password for DB
//-----------------------------------------------------------------------------
CDManager::CDManager(Options& options) : XApplication(PACKAGE " V" PRG_RELEASE), pageMenusOn(false), opt(options) {
    TRACE8("CDManager::CDManager (Options&)");

    Language::init();

    setIconProgram(picProgram, sizeof(picProgram));
    set_default_size(WIDTH, HEIGHT);

    // Create menus
    ctrlMain = Gtk::ShortcutController::create();
    ctrlMain->set_scope(Gtk::ShortcutScope::GLOBAL);
    add_controller(ctrlMain);

    Glib::RefPtr<Gio::Menu> menu(Gio::Menu::create());

    Glib::RefPtr<Gio::Menu> menuCD(Gio::Menu::create());
    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    apMenus[LOGIN] = addMenuEntry(sec, _("_Login"), "Login", sigc::mem_fun(*this, &CDManager::showLogin), _("<ctl>L"));
    apMenus[SAVE] = addMenuEntry(sec, _("_Save"), "SaveDB", sigc::mem_fun(*this, &CDManager::save));
    apMenus[LOGOUT] = addMenuEntry(sec, _("Log_out"), "Logout", sigc::mem_fun(*this, &CDManager::logout), _("<ctl>O"));
    menuCD->append_section(sec);

    sec = Gio::Menu::create();
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
    apMenus[EXPORT] =
        addMenuEntry(sec, _("_Export to HTML"), "Export", sigc::mem_fun(*this, &CDManager::export2HTML), _("<ctl>E"));
#endif
    apMenus[STATISTICS] =
        addMenuEntry(sec, _("_Information"), "Stats", sigc::mem_fun(*this, &CDManager::showStatistics), _("F12"));
    menuCD->append_section(sec);

    addMenuEntry(menuCD, _("_Quit"), "FQuit", sigc::mem_fun(*this, &CDManager::exit));
    menu->append_submenu(_("_CD"), menuCD);

    menuEdit = Gio::Menu::create();
    menu->append_submenu(_("_Edit"), menuEdit);
    menuOther = Gio::Menu::create();
    menu->append_section(menuOther);

    Glib::RefPtr<Gio::Menu> menuOptions(Gio::Menu::create());
    addMenuEntry(menuOptions, _("_Preferences"), "Prefs", sigc::mem_fun(*this, &CDManager::editPreferences), _("F9"));
    apMenus[SAVE_PREFS] = addMenuEntry(menuOptions, _("_Save preferences"), "SavePrefs",
                                       sigc::mem_fun(*this, &CDManager::savePreferences), _("<ctl>F9"));
    menu->append_submenu(_("_Options"), menuOptions);

    addHelpMenu(menu);

    enableMenus(false);

    nb.set_show_tabs((WITH_ACTORS + WITH_RECORDS + WITH_FILMS) > 1);
    nb.set_expand(true);

    getClient()->append(*Gtk::make_managed<Gtk::PopoverMenuBar>(menu));
    getClient()->append(nb);

    // Pages can add widgets next to the statusbar (see NBPage::addStatusWidget)
    Gtk::Box* boxStatus(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5));
    status.set_hexpand(true);
    boxStatus->append(status);
    getClient()->append(*boxStatus);

    try {
        const char* pLang(getenv("LANGUAGE"));
        if (!pLang) {
#ifdef HAVE_LC_MESSAGES
            pLang = setlocale(LC_MESSAGES, nullptr);
#else
            pLang = getenv("LANG");
#endif
        }
        Genres::loadFromFile(DATADIR "Genres.dat", recGenres, filmGenres, pLang);
        TRACE8("Genres: " << recGenres.size() << '/' << filmGenres.size());
    }
    catch (std::exception& e) {
        Glib::ustring msg(_("Can't read datafile containing the genres!\n\nReason: %1"));
        msg.replace(msg.find("%1"), 2, e.what());
        showError(msg);
    }

    if (opt.getUser().empty() || !login(opt.getUser(), opt.getPassword()))
        Glib::signal_idle().connect_once(sigc::mem_fun(*this, &CDManager::showLogin));

    TRACE8("CDManager::CDManager (Options&) - Add NB");
#if WITH_RECORDS == 1
    NBPage* pgRecords = (new PRecords(status, apMenus[SAVE], recGenres));
    pages[0] = pgRecords;
    nb.append_page(*Gtk::manage(pgRecords->getWindow()), _("_Records"), true);
#endif

#if WITH_FILMS == 1
    PFilms* pgFilms = (new PFilms(status, apMenus[SAVE], filmGenres));
    pages[WITH_RECORDS] = pgFilms;
    nb.append_page(*Gtk::manage(pgFilms->getWindow()), _("_Films"), true);
#endif

#if WITH_ACTORS == 1
    NBPage* pgActor = (new PActors(status, apMenus[SAVE], filmGenres, *pgFilms));
    pages[WITH_RECORDS + WITH_FILMS] = pgActor;
    nb.append_page(*Gtk::manage(pgActor->getWindow()), _("_Actors"), true);
#endif
    nb.signal_switch_page().connect(sigc::mem_fun(*this, &CDManager::pageSwitched), false);
    status.push(_("Connect to a database ..."));
    apMenus[SAVE]->set_enabled(false);

    TRACE8("CDManager::CDManager (Options&) - Show");
    show();
}

//-----------------------------------------------------------------------------
/// Adds an entry to a menu of the main window, creating the according action
/// \param menu: Menu to add the entry to
/// \param label: Label of the entry
/// \param action: Name of the action (without the "win."-prefix)
/// \param callback: Method to call, when the entry is activated
/// \param accel: Accelerator of the entry; may be empty
/// \returns Glib::RefPtr<Gio::SimpleAction> Created action
//-----------------------------------------------------------------------------
Glib::RefPtr<Gio::SimpleAction> CDManager::addMenuEntry(const Glib::RefPtr<Gio::Menu>& menu, const Glib::ustring& label,
                                                        const char* action, const sigc::slot<void()>& callback,
                                                        const Glib::ustring& accel) {
    Glib::RefPtr<Gio::SimpleAction> act(grpAction->add_action(action, callback));
    NBPage::addMenuEntry(menu, label, Glib::ustring("win.") + action, accel, ctrlMain);
    return act;
}

//-----------------------------------------------------------------------------
/// Shows a (modal) error message
/// \param msg: Message to display
/// \param title: Title of the dialog; may be empty
//-----------------------------------------------------------------------------
void CDManager::showError(const Glib::ustring& msg, const Glib::ustring& title) {
    Gtk::MessageDialog dlg(*this, msg, false, Gtk::MessageType::ERROR);
    if (title.size())
        dlg.set_title(title);
    XGP::runModal(dlg);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDManager::~CDManager() {
    TRACE8("CDManager::~CDManager ()");
    for (unsigned int i(0); i < (sizeof(pages) / sizeof(*pages)); ++i)
        pages[i]->clear();
}

//-----------------------------------------------------------------------------
/// Saves the DB
//-----------------------------------------------------------------------------
void CDManager::save() {
    TRACE9("CDManager::save ()");
    try {
        for (unsigned int i(0); i < (sizeof(pages) / sizeof(*pages)); ++i)
            if (pages[i]->isChanged())
                pages[i]->saveData();

        Check3(apMenus[SAVE]);
        apMenus[SAVE]->set_enabled(false);
    }
    catch (SaveCelebrity::DlgCanceled&) {
    }
    catch (std::exception& err) {
        Glib::ustring msg(_("Error saving data!\n\nReason: %1"));
        msg.replace(msg.find("%1"), 2, err.what());
        showError(msg);
    }
}

//-----------------------------------------------------------------------------
/// Shows the statistic-dialog
//-----------------------------------------------------------------------------
void CDManager::showStatistics() { Statistics::create(*this); }

//-----------------------------------------------------------------------------
/// Edits the preferences
//-----------------------------------------------------------------------------
void CDManager::editPreferences() { Settings::create(*this, opt); }

//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CDManager::showAboutbox() {
    std::string ver(_("Copyright (C) 2004 - 2011 Markus Schwab"
                      "\ne-mail: <g17m0@lusers.sourceforge.net>\n\nCompiled on %1 at %2"));
    ver.replace(ver.find("%1"), 2, __DATE__);
    ver.replace(ver.find("%2"), 2, __TIME__);

    XGP::XAbout* about(XGP::XAbout::create(ver, PACKAGE " V" VERSION));
    about->setIconProgram(picProgram, sizeof(picProgram));
    about->setIconAuthor(picAuthor, sizeof(picAuthor));
    about->set_transient_for(*this);
}

//-----------------------------------------------------------------------------
/// Returns the name of the file to display in the help
/// \returns \c Name of file to display
//-----------------------------------------------------------------------------
const char* CDManager::getHelpfile() { return DOCUDIR "CDManager.html"; }

//-----------------------------------------------------------------------------
/// Displays a dialog to login to the database
//-----------------------------------------------------------------------------
void CDManager::showLogin() {
    TRACE8("CDManager::showLogin ()");
    XGP::LoginDialog* dlg(XGP::LoginDialog::create(_("Database login")));
    dlg->set_transient_for(*this);
    dlg->sigLogin.connect(sigc::mem_fun(*this, &CDManager::login));

    if (opt.getUser().size())
        dlg->setUser(opt.getUser());
    else
        dlg->setCurrentUser();
    dlg->setPassword(opt.getPassword());
}

//-----------------------------------------------------------------------------
/// Enables or disables the menus according to the status of the program
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableMenus(bool enable) {
    for (unsigned int i(0); i < LAST; ++i)
        Check3(apMenus[i]);

    apMenus[LOGOUT]->set_enabled(enable);
    enablePageMenus(enable);
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
    apMenus[EXPORT]->set_enabled(enable);
#endif
    apMenus[STATISTICS]->set_enabled(enable);
    apMenus[SAVE_PREFS]->set_enabled(enable);

    nb.set_sensitive(enable);

    apMenus[LOGIN]->set_enabled(enable = !enable);
    apMenus[SAVE]->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Enables or disables the menus of the current page, by (un)registering its
/// actions (without actions the menu-entries are insensitive)
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enablePageMenus(bool enable) {
    pageMenusOn = enable;
    if (enable && grpPage)
        insert_action_group("page", grpPage);
    else
        remove_action_group("page");
}

//-----------------------------------------------------------------------------
/// Loads the database and shows its contents.
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadDatabase() {
    TRACE8("CDManager::loadDatabase () - " << nb.get_current_page());
    // Check if page is valid (at init the current page can be -1)
    if ((unsigned int)nb.get_current_page() < (sizeof(pages) / sizeof(*pages))) {
        status.pop();
        status.push(_("Reading database ..."));

        Check3(pages[nb.get_current_page()]);
        Check3(!pages[nb.get_current_page()]->isLoaded());
        pages[nb.get_current_page()]->loadData();
        pages[nb.get_current_page()]->getFocus();
    }
}

//-----------------------------------------------------------------------------
/// Callback when switching the notebook pages
/// \param iPage: Index of the newly selected page
//-----------------------------------------------------------------------------
void CDManager::pageSwitched(Gtk::Widget*, guint iPage) {
    TRACE6("CDManager::pageSwitched (Gtk::Widget*, guint) - " << iPage);
    Check1(iPage < 3);

    if (nb.get_current_page() != -1) {
        Check3(pages[nb.get_current_page()]);
        pages[nb.get_current_page()]->removeMenu();
    }
    menuEdit->remove_all();
    menuOther->remove_all();
    if (ctrlPage)
        remove_controller(ctrlPage);

    Check3(pages[iPage]);
    if (!pages[iPage]->isLoaded() && Storage::connected())
        pages[iPage]->loadData();

    grpPage = Gio::SimpleActionGroup::create();
    ctrlPage = Gtk::ShortcutController::create();
    ctrlPage->set_scope(Gtk::ShortcutScope::GLOBAL);
    add_controller(ctrlPage);
    pages[iPage]->addMenu(menuEdit, menuOther, grpPage, ctrlPage);
    enablePageMenus(pageMenusOn);

    pages[iPage]->getFocus();
}

//-----------------------------------------------------------------------------
/// Closes the main window (checking before, if changes should be saved)
//-----------------------------------------------------------------------------
void CDManager::exit() { close(); }

//-----------------------------------------------------------------------------
/// Checks if the DB has been changed and asks if it should be saved
//-----------------------------------------------------------------------------
void CDManager::querySave() {
    for (unsigned int i(0); i < (sizeof(pages) / sizeof(*pages)); ++i)
        if (pages[i]->isChanged()) {
            Gtk::MessageDialog dlg(*this, _("The data has been modified! Save those changes?"), false, Gtk::MessageType::QUESTION,
                                   Gtk::ButtonsType::YES_NO);
            dlg.set_title(PACKAGE);
            if (XGP::runModal(dlg) == Gtk::ResponseType::YES)
                save();
            break;
        }
}

//-----------------------------------------------------------------------------
/// Checks if the DB has been changed and asks if it should be saved, before
/// closing the main window
/// \returns bool: True, if closing should be prevented
//-----------------------------------------------------------------------------
bool CDManager::on_close_request() {
    querySave();
    return XApplication::on_close_request();
}

//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param user: User to connect to the DB with
/// \param pwd: Password for user
/// \returns bool: True, if login could be performed
//-----------------------------------------------------------------------------
bool CDManager::login(const Glib::ustring& user, const Glib::ustring& pwd) {
    TRACE9("CDManager::login (const Glib::ustring&, const Glib::ustring&) - " << user << '/' << pwd);

    try {
        Storage::login(DBNAME, user.c_str(), pwd.c_str());
    }
    catch (std::exception& err) {
        Glib::ustring msg(_("Can't connect to database!\n\nReason: %1"));
        msg.replace(msg.find("%1"), 2, err.what());
        showError(msg, _("Login error"));
        return false;
    }

    try {
        Storage::loadSpecialWords();
        TRACE1("CDManager::login () - Key: " << Words::getMemoryKey());
    }
    catch (std::exception& err) {
        Glib::ustring msg(_("Can't query needed information!\n\nReason: %1"));
        msg.replace(msg.find("%1"), 2, err.what());
        showError(msg);
    }

    enableMenus(true);
    loadDatabase();
    return true;
}

//-----------------------------------------------------------------------------
/// Logout from the DB; give an opportunity to save changes
//-----------------------------------------------------------------------------
void CDManager::logout() {
    TRACE8("CDManager::logout ()");
    querySave();

    for (unsigned int i(0); i < (sizeof(pages) / sizeof(*pages)); ++i)
        pages[i]->clear();

    Words::destroy();
    Storage::logout();
    enableMenus(false);
    status.pop();
    status.push(_("Disconnected!"));
}

//-----------------------------------------------------------------------------
/// Edits dthe preferences
//-----------------------------------------------------------------------------
void CDManager::savePreferences() {
    TRACE9("CDManager::savePreferences ()");

    if (opt.pINIFile) {
        TRACE5("CDManager::savePreferences () - " << opt.pINIFile);
        std::ofstream inifile(opt.pINIFile);
        if (inifile) {
            inifile << "[Database]\nUser=" << opt.getUser() << "\nPassword=" << opt.getPassword() << "\n\n";

            YGP::INIFile::write(inifile, "Export", opt);

#ifdef WITH_FILMS
            inifile << "\n[Films]\nLanguage=" << Film::currLang << '\n';
#endif
        }
        else {
            Glib::ustring msg(_("Can't create file `%1'!\n\nReason: %2."));
            msg.replace(msg.find("%1"), 2, opt.pINIFile);
            msg.replace(msg.find("%2"), 2, strerror(errno));
            showError(msg);
        }
    }

    // Storing the special/first names and the articles
    try {
        Storage::startTransaction();
        Storage::deleteNames();
        Words::forEachName(0, Words::cNames(), &Storage::storeWord);
        Storage::commitTransaction();

        Storage::startTransaction();
        Storage::deleteArticles();
        Words::forEachArticle(0, Words::cArticles(), &Storage::storeArticle);
        Storage::commitTransaction();
    }
    catch (std::exception& e) {
        Storage::abortTransaction();
        Glib::ustring msg(_("Can't store special names!\n\nReason: %1."));
        msg.replace(msg.find("%1"), 2, e.what());
        showError(msg);
    }
}

#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
//-----------------------------------------------------------------------------
/// Exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::export2HTML() {
    std::string dir(opt.getDirOutput());
    if (dir.size() && (dir[dir.size() - 1] != YGP::File::DIRSEPARATOR)) {
        dir += YGP::File::DIRSEPARATOR;
        opt.setDirOutput(dir);
    }

    // Load data
    for (unsigned int i(0); i < (WITH_RECORDS + WITH_FILMS); ++i)
        if (!pages[i]->isLoaded())
            pages[i]->loadData();

    // Get the key for the shared memory holding the special words
    std::ostringstream memKey;
    memKey << Words::getMemoryKey() << std::ends;

    const char* envLang(getenv("LANGUAGE"));
    std::string oldLang;
    if (envLang)
        oldLang = envLang;

    std::string key(memKey.str());
    const char* args[] = {"CDWriter",
                          "--outputDir",
                          opt.getDirOutput().c_str(),
#    if WITH_RECORDS == 1
                          "--recHeader",
                          opt.getRHeader().c_str(),
                          "--recFooter",
                          opt.getRFooter().c_str(),
#    endif
#    if WITH_FILMS == 1
                          "--filmHeader",
                          opt.getMHeader().c_str(),
                          "--filmFooter",
                          opt.getMFooter().c_str(),
#    endif
                          nullptr,
                          key.c_str(),
                          nullptr};
    const unsigned int POS_LANG((sizeof(args) / sizeof(*args)) - 3);
    Check2(!args[POS_LANG]);

    // Export to every language supported
    Glib::ustring statMsg(_("Exporting (language %1) ..."));
    std::string allLangs(LANGUAGES, sizeof(LANGUAGES) - 1);
    boost::tokenizer<> langs(allLangs);
    for (boost::tokenizer<>::iterator l(langs.begin()); l != langs.end(); ++l) {
        TRACE6("CDManager::export2HTML() - Lang: " << *l);
        Glib::ustring stat(statMsg);
        stat.replace(stat.find("%1"), 2, Language::findInternational(*l));
        status.push(stat);

        Glib::RefPtr<Glib::MainContext> ctx(Glib::MainContext::get_default());
        while (ctx->iteration(false))
            ; // Update statusbar

        pid_t pid(-1);
        int pipes[2];
        try {
            setenv("LANGUAGE", l->c_str(), true);
            args[POS_LANG] = l->c_str();
            TRACE3("CDManager::export2HTML() - Parms: " << args[POS_LANG] << ' ' << args[POS_LANG + 1]);

            if (pipe(pipes) < 0)
                throw std::runtime_error(strerror(errno));
            pid = YGP::Process::execIOConnected("CDWriter", args, pipes);

            for (unsigned int i(0); i < (WITH_RECORDS + WITH_FILMS); ++i)
                pages[i]->export2HTML(pipes[1], *l);
            ::close(pipes[1]);

            char output[128] = "";
            std::string allOut;
            int cRead;
            while ((cRead = ::read(pipes[0], output, sizeof(output))) != -1) {
                allOut.append(output, cRead);
                if (!cRead)
                    break;
            }
            Check3(pid != -1);
            YGP::Process::waitForProcess(pid);
            if (allOut.size()) {
                Gtk::MessageDialog dlg(*this, Glib::locale_to_utf8(allOut), false, Gtk::MessageType::INFO);
                dlg.set_title(_("Export Warning!"));
                XGP::runModal(dlg);
            }
        }
        catch (std::exception& err) {
            showError(err.what());
        }
        ::close(pipes[0]);
        status.pop();
    } // end-while
    setenv("LANGUAGE", oldLang.c_str(), true);
}
#endif
