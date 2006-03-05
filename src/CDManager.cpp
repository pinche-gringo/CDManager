//$Id: CDManager.cpp,v 1.74 2006/03/05 22:37:36 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.74 $
//AUTHOR      : Markus Schwab
//CREATED     : 10.10.2004
//COPYRIGHT   : Copyright (C) 2004 - 2006

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <cdmgr-cfg.h>

#include <cstring>
#include <cstdlib>
#include <clocale>
#include <unistd.h>

#include <fstream>
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
#include <XGP/XFileDlg.h>
#include <XGP/LoginDlg.h>

#include "Words.h"
#include "CDAppl.h"
#include "LangDlg.h"
#include "Options.h"
#include "Storage.h"
#include "Settings.h"
#include "Language.h"

#if WITH_ACTORS == 1
#  include "PActors.h"
#endif
#if WITH_MOVIES == 1
#  include <YGP/Tokenize.h>
#  include "PMovies.h"
#endif
#if WITH_RECORDS == 1
#  include "PRecords.h"
#endif

#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
#  include <YGP/Process.h>
#  include <YGP/Tokenize.h>
#endif

#include "CDManager.h"


const unsigned int CDManager::WIDTH (800);
const unsigned int CDManager::HEIGHT (600);

const char* const CDManager::DBNAME ("CDMedia");


// Pixmap for program
const char* CDManager::xpmProgram[] = {
   "48 48 65 1",
   " 	c None",
   ".	c #020300",
   "+	c #01080B",
   "@	c #0B070D",
   "#	c #160B0E",
   "$	c #10120F",
   "%	c #2C0B00",
   "&	c #1D191B",
   "*	c #2A2520",
   "=	c #591302",
   "-	c #383332",
   ";	c #4D4643",
   ">	c #903A37",
   ",	c #993E26",
   "'	c #5C5A3C",
   ")	c #5D5954",
   "!	c #825545",
   "~	c #6A6C69",
   "{	c #AB5343",
   "]	c #7A695A",
   "^	c #838160",
   "/	c #BB6852",
   "(	c #9B7C70",
   "_	c #CE6A52",
   ":	c #8D857F",
   "<	c #C8734C",
   "[	c #95926A",
   "}	c #C77B5F",
   "|	c #A08F7C",
   "1	c #DD896A",
   "2	c #A8A27C",
   "3	c #9AA3B8",
   "4	c #A8A9B3",
   "5	c #F58F6F",
   "6	c #AFACA7",
   "7	c #BDAC8E",
   "8	c #BDAB99",
   "9	c #F49685",
   "0	c #C6AAA9",
   "a	c #EC9F83",
   "b	c #DBABA3",
   "c	c #C3B7B7",
   "d	c #FBAA8E",
   "e	c #C7C1A9",
   "f	c #D1BCAC",
   "g	c #B9C4DF",
   "h	c #DFC1B7",
   "i	c #FFB897",
   "j	c #DDC3C4",
   "k	c #EFC0A7",
   "l	c #C7CEC9",
   "m	c #FCC2AA",
   "n	c #D9D3C1",
   "o	c #E4CFBE",
   "p	c #E3D6D7",
   "q	c #E1DBD6",
   "r	c #EFD8D3",
   "s	c #EFDAC9",
   "t	c #FED6C0",
   "u	c #EEDFC6",
   "v	c #DEE5DA",
   "w	c #EFEAD4",
   "x	c #FAE7E2",
   "y	c #FAEFE1",
   "z	c #F5F1EF",
   "                                                ",
   "                                                ",
   "c.  ..>{/1dimmmmddc44..  .                      ",
   "~+  @@>{{15dtutm99b64.@  @                      ",
   "~....+>>{_55imtd__dg3.....                      ",
   "~.@@.+>>>,<55dm__}agg..@..                      ",
   "~@  @.@@@@.@@@@@@@+@@..  .                      ",
   "~.  ..@@+..+.$#@+++....  @                      ",
   "~@..@+#%=,{/}11dddmg3@....                      ",
   "~..@...=,{//}1adddd33#.@..                      ",
   "~@  .#%>{}{/}1daddm33..  .                      ",
   "~.  ..>{/}////a}_}433@.  @                      ",
   "~@...+>{/}}//<}1<9433@....                      ",
   "~..@@.{{}1aaa<amad344.@@..                      ",
   "~@  +.>{/1diktmmdac44..  .                      ",
   "~.  .+>{{15itttt99bc4.. .#&@.@&                 ",
   "~@.@.+>>{_55imtd__d33..*-;:bn8^)-*#             ",
   "~....+>>>,/55ii__}bg3$-|fb0oyf80|:)$            ",
   "~@  .+++@..@@@@+@##@&:ssuh0hof887hh^*$          ",
   "~.  ..#@.@.+.+++++$*:zzwso0hsf8(fff2^*$         ",
   "~@..@+#%=,{/}1add}-0rxxzuo0jsf68fo72[^*#        ",
   "~..@.#.=,{//}11d9;:xrxxxsuhhof78u72[[[^$        ",
   "~@  .@%,{////1da()jxrrxxxuocs88fe2[[[[7]&       ",
   "~.  ..>><}/}/}1}-jxxxxrrzzo0o88f7[[^2b8b*       ",
   "~@...+>{/}}//<}!)syxxxxrrxzzzzqo[[[7k0rs|&      ",
   "~..@@+{{}1aa1}a!|uutyxyxrzzzzqzv7[8hhsuyu*      ",
   "*+  ++>{/1ditkk;8hhsssuyzzl)-;:vzkhwwwwvv)&     ",
   "              @-0b00bhtuzz)&***6zywwwvvvq~+     ",
   "               ;0hhhh0jry6-&-&-:zzlvlllll:+     ",
   "               ;osysosopzc-&*--~zveeelele~+     ",
   "               ;effff878zc)----lzzle7e7e7)#     ",
   "               -|888888fqz4);-6zzppqvnlee;$     ",
   "               +]860|6fe2vvzzzzzwsrppqqvl$@     ",
   "               @;|77hoe2[[qzzzzrpurrrrpq:$      ",
   "                +|hof72[28rvlelsruuoqprj-&      ",
   "                &;ee72[[[8svleelrrouurr:*@      ",
   "                 &]22[[[7huvvlelrrruuuf&@       ",
   "                 ##^[[[|7hywle7nprrsue;##       ",
   "                  &$'[[8hrwvll7elprrh*&+        ",
   "                   .@'[khuwqlllelpj:-&.         ",
   "                    .@&(fwwvlle7e4)#$$          ",
   "                     ##&-~8ll6[);*###           ",
   "                       @+&&+++$+&@+             ",
   "                         $.++@+++               ",
   "                                                ",
   "                                                ",
   "                                                ",
   "                                                "};

// Pixmap for author
const char* CDManager::xpmAuthor[] = {
   "56 43 5 1",
   " 	c None",
   "!   c #0000FF",
   "@	c #000000",
   "-	c #AEAAAE",
   ".	c #FFFFFF",
   ".......................................................@",
   "..-----------------------------------------------------@",
   "..-----------------------------------------------------@",
   "..----------------------!!!!!!-------------------------@",
   "..--------------------!!!!@@!!!!-----------------------@",
   "..------------------!!!!@@---@@!!!---------------------@",
   "..-----------------!!!@@--------!!!--------------------@",
   "..-----------------!!!@---!!----!!!--------------------@",
   "..----------------!!!@---!!!!@---!!!-------------------@",
   "..----------------!!@---!!!!!!@---!!@------------------@",
   "..----------------!!@----!!!!@----!!@------------------@",
   "..----------------!!!-----!!@----!!!@------------------@",
   "..-----------------!!!-----@-----!!!@------------------@",
   "..-----------------!!!!---------!!!@-------------------@",
   "..------------------!!!!-------!!!@--------------------@",
   "..--------------------!!!!--!!!!@@---------------------@",
   "..----------------------!!!!!!@@-----------------------@",
   "..-----------------------!!!!@-------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!!!!!!!!!!!!!!--------------------@",
   "..----------------!!!@@@@@!!@@@@@!!!-------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..-----------------@@-----!!@------@@------------------@",
   "..------------------------!!@--------------------------@",
   "..--------------------!!!!!!!!!!-----------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!@@@@@@@@@!!!!--------------------@",
   "..-----------------!!@----------!!!!-------------------@",
   "..----------------!!!@-----------!!!@------------------@",
   "..----------------!!@------------!!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!@--------------!!@------------------@",
   "..----------------!!@--------------@@------------------@",
   "..-----------------!@----------------------------------@",
   "..------------------@----------------------------------@",
   "..-----------------------------------------------------@",
   "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" };


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

   setIconProgram (xpmProgram);
   set_default_size (WIDTH, HEIGHT);

   // Create controls
   Glib::ustring ui ("<ui><menubar name='Menu'>"
		     "  <menu action='CD'>"
		     "    <menuitem action='Login'/>"
		     "    <menuitem action='SaveDB'/>"
		     "    <menuitem action='Logout'/>"
#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
		     "    <separator/>"
		     "    <menuitem action='Export'/>"
#endif
#if WITH_RECORDS == 1
		     "    <menuitem action='Import'/>"
#endif
		     "    <separator/>"
		     "    <menuitem action='FQuit'/>"
		     "  </menu>"
		     "  <menu action='Edit'>"
		     "    <placeholder name='EditAction'/>"
		     "  </menu>"
		     "  <placeholder name='Lang'/>"
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
#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
   grpAction->add (apMenus[EXPORT] = Gtk::Action::create ("Export", _("_Export to HTML")),
		   Gtk::AccelKey (_("<ctl>E")),
		   mem_fun (*this, &CDManager::export2HTML));
#endif
#if (WITH_RECORDS == 1)
   grpAction->add (apMenus[IMPORT_MP3] = Gtk::Action::create ("Import", _("_Import from file-info ...")),
		   Gtk::AccelKey (_("<ctl>I")),
		   mem_fun (*this, &CDManager::importFromFileInfo));
#endif
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

   nb.set_show_tabs (WITH_ACTORS + WITH_RECORDS + WITH_MOVIES - 1);

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
      Genres::loadFromFile (DATADIR "Genres.dat", recGenres, movieGenres, pLang);
      TRACE8 ("Genres: " << recGenres.size () << '/' << movieGenres.size ());
   }
   catch (std::string& e) {
      Glib::ustring msg (_("Can't read datafile containing the genres!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, e);
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

#if WITH_MOVIES == 1
   PMovies* pgMovies = (new PMovies (status, apMenus[SAVE], movieGenres));
   pages[WITH_RECORDS] = pgMovies;
   nb.append_page (*manage (pgMovies->getWindow ()), _("_Movies"), true);
#endif

#if WITH_ACTORS == 1
   NBPage* pgActor = (new PActors (status, apMenus[SAVE], movieGenres, *pgMovies));
   pages[WITH_RECORDS + WITH_MOVIES] = pgActor;
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
   try {
      for (unsigned int i (0); i < (sizeof (pages) / sizeof (*pages)); ++i)
	 if (pages[i]->isChanged ())
	    pages[i]->saveData ();

      apMenus[SAVE]->set_sensitive (false);
   }
   catch (Glib::ustring& msg) {
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

#if WITH_RECORDS == 1
//-----------------------------------------------------------------------------
/// Imports information from audio file (e.g. MP3-ID3 tag or OGG-commentheader)
//-----------------------------------------------------------------------------
void CDManager::importFromFileInfo () {
   XGP::TFileDialog<CDManager>::create (_("Select file(s) to import"), *this,
					&CDManager::parseFileInfo,
					Gtk::FILE_CHOOSER_ACTION_OPEN,
					XGP::IFileDialog::MUST_EXIST
					| XGP::IFileDialog::MULTIPLE);
}
#endif

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
   std::string ver (_("Copyright (C) 2004 - 2006 Markus Schwab"
                      "\ne-mail: <g17m0@lycos.com>\n\nCompiled on %1 at %2"));
   ver.replace (ver.find ("%1"), 2, __DATE__);
   ver.replace (ver.find ("%2"), 2, __TIME__);

   XGP::XAbout* about (XGP::XAbout::create (ver, PACKAGE " V" VERSION));
   about->setIconProgram (xpmProgram);
   about->setIconAuthor (xpmAuthor);
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
#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
   apMenus[EXPORT]->set_sensitive (enable);
#endif
#if WITH_RECORDS == 1
   apMenus[IMPORT_MP3]->set_sensitive (enable);
#endif
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

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   pages[iPage]->addMenu (ui, grpAction);

   ui += "</menubar>";
   mgrUI->insert_action_group (grpAction);
   idPageMrg = mgrUI->add_ui_from_string (ui);

   Check3 (pages[iPage]);
   if (!pages[iPage]->isLoaded () && Storage::connected ())
      pages[iPage]->loadData ();
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
   std::ofstream inifile (opt.pINIFile);
   if (inifile) {
      inifile << "[Database]\nUser=" << opt.getUser () << "\nPassword="
	      << opt.getPassword () << "\n\n";

      YGP::INIFile::write (inifile, "Export", opt);

#ifdef HAVE_MOVIES
      inifile << "[Movies]\nLanguage=" << Movie::currLang << '\n';
#endif
   }
   else {
      Glib::ustring msg (_("Can't create file `%1'!\n\nReason: %2."));
      msg.replace (msg.find ("%1"), 2, opt.pINIFile);
      msg.replace (msg.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
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

#if (WITH_RECORDS == 1) || (WITH_MOVIES == 1)
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
   for (unsigned int i (0); i < (WITH_RECORDS + WITH_MOVIES); ++i)
      if (!pages[i]->isLoaded ())
	 pages[i]->loadData ();

   // Get the key for the shared memory holding the special words
   std::ostringstream memKey;
   memKey << Words::getMemoryKey () << std::ends;

   YGP::Tokenize langs (LANGUAGES);
   std::string lang;

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
#if WITH_MOVIES == 1
			  "--movieHeader", opt.getMHeader ().c_str (),
			  "--movieFooter", opt.getMFooter ().c_str (),
#endif
			  NULL, key.c_str (), NULL };
   const unsigned int POS_LANG ((sizeof (args) / sizeof (*args)) - 3);
   Check2 (!args[POS_LANG]);

   // Export to every language supported
   Glib::ustring statMsg (_("Exporting (language %1) ..."));
   while ((lang = langs.getNextNode (' ')).size ()) {
      TRACE6 ("CDManager::export2HTML () - Lang: " << lang);
      Glib::ustring stat (statMsg);
      stat.replace (stat.find ("%1"), 2, Language::findInternational (lang));
      status.push (stat);

      Glib::RefPtr<Glib::MainContext> ctx (Glib::MainContext::get_default ());
      while (ctx->iteration (false));                      // Update statusbar

      pid_t pid (-1);
      int pipes[2];
      try {
	 setenv ("LANGUAGE", lang.c_str (), true);
	 args[POS_LANG] = lang.c_str ();
	 TRACE3 ("CDManager::export2HTML () - Parms: " << args[POS_LANG] << ' ' << args[POS_LANG + 1]);

	 pipe (pipes);
	 pid = YGP::Process::execIOConnected ("CDWriter", args, pipes);

	 for (unsigned int i (0); i < (WITH_RECORDS + WITH_MOVIES); ++i)
	    pages[i]->export2HTML (pipes[1], lang);
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
      catch (Glib::ustring& e) {
	 Gtk::MessageDialog dlg (e, Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
      catch (std::string& e) {
	 Gtk::MessageDialog dlg (Glib::locale_to_utf8 (e), Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
      catch (Glib::Error& e) {
	 Gtk::MessageDialog dlg (e.what (), Gtk::MESSAGE_ERROR);
	 dlg.run ();
      }
      close (pipes[0]);
      status.pop ();
   } // end-while
   setenv ("LANGUAGE", oldLang.c_str (), true);
}
#endif

#if WITH_RECORDS == 1
//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param file: Name of file to analzye
//-----------------------------------------------------------------------------
void CDManager::parseFileInfo (const std::string& file) {
   TRACE8 ("CDManager::parseFileInfo (const std::string&) - " << file);
   Check2 (file.size ());

   std::ifstream stream (file.c_str ());
   Glib::ustring artist, record, song;
   unsigned int track (0);
   if (!stream) {
      Glib::ustring err (_("Can't open file `%1'!\n\nReason: %2"));
      err.replace (err.find ("%1"), 2, file);
      err.replace (err.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog (err).run ();
      return;
   }

   TRACE1 ("CDManager::parseFileInfo (const std::string&) - Type: " << file.substr (file.size () - 4));
   if (((file.substr (file.size () - 4) == ".mp3")
	&& parseMP3Info (stream, artist, record, song, track))
       || ((file.substr (file.size () - 4) == ".ogg")
	   && parseOGGCommentHeader (stream, artist, record, song, track))) {
      TRACE8 ("CDManager::parseFileInfo (const std::string&) - " << artist
	      << '/' << record << '/' << song << '/' << track);
      Check1 (typeid (**pages) == typeid (PRecords));
      ((PRecords*)*pages)->addEntry (artist, record, song, track);
   }
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param stream: MP3-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if ID3 info has been found
//-----------------------------------------------------------------------------
bool CDManager::parseMP3Info (std::istream& stream, Glib::ustring& artist,
			      Glib::ustring& record, Glib::ustring& song,
			      unsigned int& track) {
   stream.seekg (-0x80, std::ios::end);
   std::string value;

   std::getline (stream, value, '\xff');
   TRACE8 ("CDManager::parseMP3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Found: "
	   << value << "; Length: " << value.size ());
   if ((value.size () > 3) && (value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
      song = Glib::locale_to_utf8 (stripString (value, 3, 29));
      artist = Glib::locale_to_utf8 (stripString (value, 33, 29));
      record = Glib::locale_to_utf8 (stripString (value, 63, 29));
      track = (value[0x7d] != 0x20) ? value[0x7e] : 0;
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Reads the OGG comment header from an OGG vorbis encoded file
/// \param stream: OGG-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if comment header has been found
//-----------------------------------------------------------------------------
bool CDManager::parseOGGCommentHeader (std::istream& stream, Glib::ustring& artist,
				       Glib::ustring& record, Glib::ustring& song,
				       unsigned int& track) {
   char buffer[512];
   stream.read (buffer, 4);
   if ((*buffer != 'O') && (buffer[1] != 'g') && (buffer[2] != 'g') && (buffer[3] != 'S'))
      return false;

   stream.seekg (0x69, std::ios::cur);
   unsigned int len (0);
   stream.read ((char*)&len, 4);                // Read the vendorstring-length
   TRACE8 ("CDManager::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Length: " << len);
   stream.seekg (len, std::ios::cur);

   unsigned int cComments (0);
   stream.read ((char*)&cComments, 4);               // Read number of comments
   TRACE8 ("CDManager::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Comments: " << cComments);
   if (!cComments)
      return false;

   std::string key;
   Glib::ustring *value (NULL);
   do {
      stream.read ((char*)&len, 4);                  // Read the comment-length

      std::getline (stream, key, '=');
      len -= key.size () + 1;
      TRACE8 ("CDManager::parseOGGCommentHeader (std::stream&, 3x Glib::ustring&, unsigned&) - Key: " << key);

      if (key == "TITLE")
	 value = &song;
      else if (key == "ALBUM")
	 value = &record;
      else if (key == "ARTIST")
	 value = &artist;
      else if (key == "TRACKNUMBER") {
	 Check2 (len < sizeof (buffer));
	 stream.read (buffer, len);
	 track = strtoul (buffer, NULL, 10);
	 value = NULL;
	 len = 0;
      }
      else
	 value = NULL;

      if (value) {
	 unsigned int read (0);
	 do {
	    read = stream.readsome (buffer, (len > sizeof (buffer)) ? sizeof (buffer) - 1 : len);
	    len -= read;
	    buffer[read] = '\0';
	    value->append (buffer);
 	 } while (len);
      }
      else
	 stream.seekg (len, std::ios::cur);
   } while (--cComments);  // end-do while comments
   return true;
}

//-----------------------------------------------------------------------------
/// Returns the specified substring, removed from trailing spaces
/// \param value: String to manipulate
/// \param pos: Starting pos inside the string
/// \param len: Maximal length of string
/// \returns std::string: Stripped value
//-----------------------------------------------------------------------------
std::string CDManager::stripString (const std::string& value, unsigned int pos, unsigned int len) {
   len += pos;
   while (len > pos) {
      if ((value[len] != ' ') && (value[len]))
         break;
      --len;
   }
   return (pos == len) ? " " : value.substr (pos, len - pos + 1);
}

#endif
