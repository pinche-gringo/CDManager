//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 10.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2011

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

#include <cstring>
#include <cstdlib>
#include <clocale>
#include <unistd.h>

#include <sstream>

#include <gdkmm/pixbuf.h>

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>

// TRACELEVEL 1 shows shared-memory key; TRACELEVEL 9 shows password
#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>

#include <XGP/XAbout.h>
#include <XGP/LoginDlg.h>

#include "Words.h"
#include "CDAppl.h"
#include "LangDlg.h"
#include "Options.h"
#include "Storage.h"
#include "Settings.h"
#include "Language.h"
#include "SaveCeleb.h"
#include "Statistics.h"

#if WITH_ACTORS == 1
#  include "PActors.h"
#endif
#if WITH_FILMS == 1
#  include <boost/tokenizer.hpp>
#  include "PFilms.h"
#endif
#if WITH_RECORDS == 1
#  include "PRecords.h"
#endif

#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
#  include <YGP/Process.h>
#  include <boost/tokenizer.hpp>
#endif

#include "CDManager.h"


const unsigned int CDManager::WIDTH (800);
const unsigned int CDManager::HEIGHT (600);

const char* const CDManager::DBNAME ("CDMedia");


#include "IconAuthor.h"
#include "IconProgram.h"

//-----------------------------------------------------------------------------
/// Defaultconstructor; all widget are created
/// \param options: Options for the program
/// \param user: User for database
/// \param pwd: Password for DB
//-----------------------------------------------------------------------------
CDManager::CDManager (Options& options)
   : XApplication (PACKAGE " V" PRG_RELEASE),
     opt (options) {
   TRACE8 ("CDManager::CDManager (Options&)");

   Language::init ();

   setIconProgram (picProgram, sizeof (picProgram));
   set_default_size (WIDTH, HEIGHT);

   // Create controls
   Glib::ustring ui ("<ui><menubar name='Menu'>"
		     "  <menu action='CD'>"
		     "    <menuitem action='Login'/>"
		     "    <menuitem action='SaveDB'/>"
		     "    <menuitem action='Logout'/>"
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
		     "    <separator/>"
		     "    <menuitem action='Export'/>"
#endif
		     "    <menuitem action='Stats'/>"
		     "    <separator/>"
		     "    <menuitem action='FQuit'/>"
		     "  </menu>"
		     "  <menu action='Edit'>"
		     "    <placeholder name='EditAction'/>"
		     "  </menu>"
		     "  <placeholder name='Other'/>"
		     "  <menu action='Options'>"
		     "    <menuitem action='Prefs'/>"
		     "    <menuitem action='SavePrefs'/>"
		     "  </menu>");

   grpAction->add (Gtk::Action::create ("CD", _("_CD")));
   grpAction->add (apMenus[LOGIN] = Gtk::Action::create ("Login", _("_Login")),
		   Gtk::AccelKey (_("<ctl>L")),
		   mem_fun (*this, &CDManager::showLogin));
   grpAction->add (apMenus[SAVE] = Gtk::Action::create ("SaveDB", Gtk::Stock::SAVE),
		   mem_fun (*this, &CDManager::save));
   grpAction->add (apMenus[LOGOUT] = Gtk::Action::create ("Logout", _("Log_out")),
		   Gtk::AccelKey (_("<ctl>O")),
		   mem_fun (*this, &CDManager::logout));
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
   grpAction->add (apMenus[EXPORT] = Gtk::Action::create ("Export", _("_Export to HTML")),
		   Gtk::AccelKey (_("<ctl>E")),
		   mem_fun (*this, &CDManager::export2HTML));
#endif
   grpAction->add (apMenus[STATISTICS] = Gtk::Action::create ("Stats", Gtk::Stock::INFO),
		   Gtk::AccelKey (_("F12")),
		   mem_fun (*this, &CDManager::showStatistics));
   grpAction->add (Gtk::Action::create ("FQuit", Gtk::Stock::QUIT),
		   mem_fun (*this, &CDManager::exit));
   grpAction->add (apMenus[MEDIT] = Gtk::Action::create ("Edit", _("_Edit")));
   grpAction->add (Gtk::Action::create ("Options", _("_Options")));
   grpAction->add (Gtk::Action::create ("Prefs", Gtk::Stock::PREFERENCES),
		   Gtk::AccelKey (_("F9")),
		   mem_fun (*this, &CDManager::editPreferences));
   grpAction->add (apMenus[SAVE_PREFS] = Gtk::Action::create ("SavePrefs", _("_Save preferences")),
		   Gtk::AccelKey (_("<ctl>F9")),
		   mem_fun (*this, &CDManager::savePreferences));

   addHelpMenu (ui);
   ui += "</menubar></ui>";
   mgrUI->insert_action_group (grpAction);
   add_accel_group (mgrUI->get_accel_group ());
   mgrUI->add_ui_from_string (ui);

   Check3 (mgrUI->get_widget("/Menu/Help"));
   ((Gtk::MenuItem*)(mgrUI->get_widget("/Menu/Help")))->set_right_justified ();

   enableMenus (false);

   nb.set_show_tabs (WITH_ACTORS + WITH_RECORDS + WITH_FILMS - 1);

   getClient ()->pack_start (*mgrUI->get_widget("/Menu"), Gtk::PACK_SHRINK);
   getClient ()->pack_start (nb, Gtk::PACK_EXPAND_WIDGET);
   getClient ()->pack_end (status, Gtk::PACK_SHRINK);

   try {
      const char* pLang (getenv ("LANGUAGE"));
      if (!pLang) {
#ifdef HAVE_LC_MESSAGES
	 pLang = setlocale (LC_MESSAGES, NULL);
#else
	 pLang = getenv ("LANG");
#endif
      }
      Genres::loadFromFile (DATADIR "Genres.dat", recGenres, filmGenres, pLang);
      TRACE8 ("Genres: " << recGenres.size () << '/' << filmGenres.size ());
   }
   catch (std::exception& e) {
      Glib::ustring msg (_("Can't read datafile containing the genres!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, e.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }

   if (opt.getUser ().empty ()
       || !login (opt.getUser (), opt.getPassword ()))
      Glib::signal_idle ().connect
	 (bind_return (mem_fun (*this, &CDManager::showLogin), false));

   TRACE8 ("CDManager::CDManager (Options&) - Add NB");
#if WITH_RECORDS == 1
   NBPage* pgRecords = (new PRecords (status, apMenus[SAVE], recGenres));
   pages[0] = pgRecords;
   nb.append_page (*manage (pgRecords->getWindow ()), _("_Records"), true);
#endif

#if WITH_FILMS == 1
   PFilms* pgFilms = (new PFilms (status, apMenus[SAVE], filmGenres));
   pages[WITH_RECORDS] = pgFilms;
   nb.append_page (*manage (pgFilms->getWindow ()), _("_Films"), true);
#endif

#if WITH_ACTORS == 1
   NBPage* pgActor = (new PActors (status, apMenus[SAVE], filmGenres, *pgFilms));
   pages[WITH_RECORDS + WITH_FILMS] = pgActor;
   nb.append_page (*manage (pgActor->getWindow ()), _("_Actors"), true);
#endif
   nb.signal_switch_page ().connect (mem_fun (*this, &CDManager::pageSwitched), false);
   status.push (_("Connect to a database ..."));
   apMenus[SAVE]->set_sensitive (false);

   TRACE8 ("CDManager::CDManager (Options&) - Show");
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDManager::~CDManager () {
   TRACE8 ("CDManager::~CDManager ()");
   for (unsigned int i (0); i < (sizeof (pages) / sizeof (*pages)); ++i)
      pages[i]->clear ();
}

//-----------------------------------------------------------------------------
/// Saves the DB
//-----------------------------------------------------------------------------
void CDManager::save () {
   TRACE9 ("CDManager::save ()");
   try {
      for (unsigned int i (0); i < (sizeof (pages) / sizeof (*pages)); ++i)
	 if (pages[i]->isChanged ())
	    pages[i]->saveData ();

      Check3 (apMenus[SAVE]);
      apMenus[SAVE]->set_sensitive (false);
   }
   catch (SaveCelebrity::DlgCanceled&) {
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Error saving data!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Shows the statistic-dialog
//-----------------------------------------------------------------------------
void CDManager::showStatistics () {
   Statistics::create (get_window ());
}

//-----------------------------------------------------------------------------
/// Edits the preferences
//-----------------------------------------------------------------------------
void CDManager::editPreferences () {
   Settings::create (get_window (), opt);
}

//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CDManager::showAboutbox () {
   std::string ver (_("Copyright (C) 2004 - 2011 Markus Schwab"
                      "\ne-mail: <g17m0@lusers.sourceforge.net>\n\nCompiled on %1 at %2"));
   ver.replace (ver.find ("%1"), 2, __DATE__);
   ver.replace (ver.find ("%2"), 2, __TIME__);

   XGP::XAbout* about (XGP::XAbout::create (ver, PACKAGE " V" VERSION));
   about->setIconProgram (picProgram, sizeof (picProgram));
   about->setIconAuthor (picAuthor, sizeof (picAuthor));
   about->get_window ()->set_transient_for (get_window ());
}

//-----------------------------------------------------------------------------
/// Returns the name of the file to display in the help
/// \returns \c Name of file to display
//-----------------------------------------------------------------------------
const char* CDManager::getHelpfile () {
   return DOCUDIR "CDManager.html";
}

//-----------------------------------------------------------------------------
/// Displays a dialog to login to the database
//-----------------------------------------------------------------------------
void CDManager::showLogin () {
   TRACE8 ("CDManager::showLogin ()");
   XGP::LoginDialog* dlg (XGP::LoginDialog::create (_("Database login")));
   dlg->get_window ()->set_transient_for (get_window ());
   dlg->sigLogin.connect (mem_fun (*this, &CDManager::login));

   if (opt.getUser ().size ())
      dlg->setUser (opt.getUser ());
   else
      dlg->setCurrentUser ();
   dlg->setPassword (opt.getPassword ());
}

//-----------------------------------------------------------------------------
/// Enables or disables the menus according to the status of the program
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableMenus (bool enable) {
   apMenus[LOGOUT]->set_sensitive (enable);
   apMenus[MEDIT]->set_sensitive (enable);
#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
   apMenus[EXPORT]->set_sensitive (enable);
#endif
   apMenus[STATISTICS]->set_sensitive (enable);
   apMenus[SAVE_PREFS]->set_sensitive (enable);

   nb.set_sensitive (enable);

   apMenus[LOGIN]->set_sensitive (enable = !enable);
   apMenus[SAVE]->set_sensitive (false);
}

//-----------------------------------------------------------------------------
/// Loads the database and shows its contents.
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadDatabase () {
   TRACE8 ("CDManager::loadDatabase () - " << nb.get_current_page ());
   // Check if page is valid (at init the current page can be -1)
   if ((unsigned int)nb.get_current_page () < (sizeof (pages) / sizeof (*pages))) {
      status.pop ();
      status.push (_("Reading database ..."));

      Check3 (pages[nb.get_current_page ()]);
      Check3 (!pages[nb.get_current_page ()]->isLoaded ());
      pages[nb.get_current_page ()]->loadData ();
      pages[nb.get_current_page ()]->getFocus ();
   }
}

//-----------------------------------------------------------------------------
/// Callback when switching the notebook pages
/// \param iPage: Index of the newly selected page
//-----------------------------------------------------------------------------
void CDManager::pageSwitched (GtkNotebookPage*, guint iPage) {
   TRACE6 ("CDManager::pageSwitched (GtkNotebookPage*, guint) - " << iPage);
   Check1 (iPage < 3);

   static Gtk::UIManager::ui_merge_id idPageMrg (-1U);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <menu action='Edit'>"
		     "    <placeholder name='EditAction'>");

   if (nb.get_current_page () != -1) {
      Check3 (pages[nb.get_current_page ()]);
      pages[nb.get_current_page ()]->removeMenu ();
   }
   if (idPageMrg != -1U)
      mgrUI->remove_ui (idPageMrg);

   Check3 (pages[iPage]);
   if (!pages[iPage]->isLoaded () && Storage::connected ())
      pages[iPage]->loadData ();

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   pages[iPage]->addMenu (ui, grpAction);

   ui += "</menubar>";
   mgrUI->insert_action_group (grpAction);
   idPageMrg = mgrUI->add_ui_from_string (ui);

   pages[iPage]->getFocus ();
}

//-----------------------------------------------------------------------------
/// Checks if the DB has been change and asks if it should be safed, before
/// hiding (closing) the main window
//-----------------------------------------------------------------------------
void CDManager::exit () {
   on_delete_event (NULL);
   hide ();
}

//-----------------------------------------------------------------------------
/// Checks if the DB has been changed and asks if it should be safed, before
/// hiding (closing) the main window
/// \param ev: Event
/// \returns bool: True, if message has been processed
//-----------------------------------------------------------------------------
bool CDManager::on_delete_event (GdkEventAny* ev) {
   for (unsigned int i (0); i < (sizeof (pages) / sizeof (*pages)); ++i)
      if (pages[i]->isChanged ()) {
	 Gtk::MessageDialog dlg (_("The data has been modified! Save those changes?"),
				 false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
	 dlg.set_title (PACKAGE);
	 if (dlg.run () == Gtk::RESPONSE_YES)
	    save ();
	 break;
      }
   return ev ? XApplication::on_delete_event (ev) : true;
}

//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param user: User to connect to the DB with
/// \param pwd: Password for user
/// \returns bool: True, if login could be performed
//-----------------------------------------------------------------------------
bool CDManager::login (const Glib::ustring& user, const Glib::ustring& pwd) {
   TRACE9 ("CDManager::login (const Glib::ustring&, const Glib::ustring&) - " << user << '/' << pwd);

   try {
      Storage::login (DBNAME, user.c_str (), pwd.c_str ());
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't connect to database!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Login error"));
      dlg.run ();
      return false;
   }

   try {
      Storage::loadSpecialWords ();
      TRACE1 ("CDManager::login () - Key: " << Words::getMemoryKey ());
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query needed information!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }

   enableMenus (true);
   loadDatabase ();
   return true;
}

//-----------------------------------------------------------------------------
/// Logout from the DB; give an opportunity to save changes
//-----------------------------------------------------------------------------
void CDManager::logout () {
   TRACE8 ("CDManager::logout ()");
   on_delete_event (NULL);

   for (unsigned int i (0); i < (sizeof (pages) / sizeof (*pages)); ++i)
      pages[i]->clear ();

   Words::destroy ();
   Storage::logout ();
   enableMenus (false);
   status.pop ();
   status.push (_("Disconnected!"));
}

//-----------------------------------------------------------------------------
/// Edits dthe preferences
//-----------------------------------------------------------------------------
void CDManager::savePreferences () {
   TRACE9 ("CDManager::savePreferences ()");

   if (opt.pINIFile) {
      TRACE5 ("CDManager::savePreferences () - " << opt.pINIFile);
      std::ofstream inifile (opt.pINIFile);
      if (inifile) {
	 inifile << "[Database]\nUser=" << opt.getUser () << "\nPassword="
		 << opt.getPassword () << "\n\n";

	 YGP::INIFile::write (inifile, "Export", opt);

#ifdef WITH_FILMS
	 inifile << "\n[Films]\nLanguage=" << Film::currLang << '\n';
#endif
      }
      else {
	 Glib::ustring msg (_("Can't create file `%1'!\n\nReason: %2."));
	 msg.replace (msg.find ("%1"), 2, opt.pINIFile);
	 msg.replace (msg.find ("%2"), 2, strerror (errno));
	 Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
   }

   // Storing the special/first names and the articles
   try {
      Storage::startTransaction ();
      Storage::deleteNames ();
      Words::forEachName (0, Words::cNames (), &Storage::storeWord);
      Storage::commitTransaction ();

      Storage::startTransaction ();
      Storage::deleteArticles ();
      Words::forEachArticle (0, Words::cArticles (), &Storage::storeArticle);
      Storage::commitTransaction ();
   }
   catch (std::exception& e) {
      Storage::abortTransaction ();
      Glib::ustring msg (_("Can't store special names!\n\nReason: %1."));
      msg.replace (msg.find ("%1"), 2, e.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

#if (WITH_RECORDS == 1) || (WITH_FILMS == 1)
//-----------------------------------------------------------------------------
/// Exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::export2HTML () {
   std::string dir (opt.getDirOutput ());
   if (dir.size () && (dir[dir.size () - 1] != YGP::File::DIRSEPARATOR)) {
      dir += YGP::File::DIRSEPARATOR;
      opt.setDirOutput (dir);
   }

   // Load data
   for (unsigned int i (0); i < (WITH_RECORDS + WITH_FILMS); ++i)
      if (!pages[i]->isLoaded ())
	 pages[i]->loadData ();

   // Get the key for the shared memory holding the special words
   std::ostringstream memKey;
   memKey << Words::getMemoryKey () << std::ends;

   const char* envLang (getenv ("LANGUAGE"));
   std::string oldLang;
   if (envLang)
      oldLang = envLang;

   std::string key (memKey.str ());
   const char* args[] = { "CDWriter", "--outputDir", opt.getDirOutput ().c_str (),
#if WITH_RECORDS == 1
			  "--recHeader", opt.getRHeader ().c_str (),
			  "--recFooter", opt.getRFooter ().c_str (),
#endif
#if WITH_FILMS == 1
			  "--filmHeader", opt.getMHeader ().c_str (),
			  "--filmFooter", opt.getMFooter ().c_str (),
#endif
			  NULL, key.c_str (), NULL };
   const unsigned int POS_LANG ((sizeof (args) / sizeof (*args)) - 3);
   Check2 (!args[POS_LANG]);

   // Export to every language supported
   Glib::ustring statMsg (_("Exporting (language %1) ..."));
   std::string allLangs (LANGUAGES, sizeof (LANGUAGES) - 1);
   boost::tokenizer<> langs (allLangs);
   for (boost::tokenizer<>::iterator l (langs.begin ()); l != langs.end (); ++l) {
      TRACE6 ("CDManager::export2HTML () - Lang: " << *l);
      Glib::ustring stat (statMsg);
      stat.replace (stat.find ("%1"), 2, Language::findInternational (*l));
      status.push (stat);

      Glib::RefPtr<Glib::MainContext> ctx (Glib::MainContext::get_default ());
      while (ctx->iteration (false));                      // Update statusbar

      pid_t pid (-1);
      int pipes[2];
      try {
	 setenv ("LANGUAGE", l->c_str (), true);
	 args[POS_LANG] = l->c_str ();
	 TRACE3 ("CDManager::export2HTML () - Parms: " << args[POS_LANG] << ' ' << args[POS_LANG + 1]);

	 if (pipe (pipes) < 0)
	    throw std::runtime_error (strerror (errno));
	 pid = YGP::Process::execIOConnected ("CDWriter", args, pipes);

	 for (unsigned int i (0); i < (WITH_RECORDS + WITH_FILMS); ++i)
	    pages[i]->export2HTML (pipes[1], *l);
	 close (pipes[1]);

	 char output[128] = "";
	 std::string allOut;
	 int cRead;
	 while ((cRead = ::read (pipes[0], output, sizeof (output))) != -1) {
	    allOut.append (output, cRead);
	    if (!cRead)
	       break;
	 }
	 if (allOut.size ()) {
	    Gtk::MessageDialog dlg (Glib::locale_to_utf8 (output), Gtk::MESSAGE_INFO);
	    dlg.set_title (_("Export Warning!"));
	    dlg.run ();
	 }
	 Check3 (pid != -1);
	 YGP::Process::waitForProcess (pid);
      }
      catch (std::exception& e) {
	 Gtk::MessageDialog dlg (e.what (), Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
      close (pipes[0]);
      status.pop ();
   } // end-while
   setenv ("LANGUAGE", oldLang.c_str (), true);
}
#endif
