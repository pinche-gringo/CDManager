//$Id: CDManager.cpp,v 1.3 2004/10/22 03:47:47 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        : 
//BUGS        :
//REVISION    : $Revision: 1.3 $
//AUTHOR      : Markus Schwab
//CREATED     : 10.10.2004
//COPYRIGHT   : Copyright (C) 2004

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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Process.h>
#include <YGP/Tokenize.h>
#include <XGP/XFileDlg.h>
#include <XGP/LoginDlg.h>
#include "CDAppl.h"
#include "Settings.h"
#include "DB.h"
#include "Record.h"
#include "Interpret.h"

#include "RecEdit.h"


// Defines
const unsigned int CDManager::WIDTH (760);
const unsigned int CDManager::HEIGHT (750);


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
// With a very ugly trick initialize I18n before the first use of gettext)
XGP::XApplication::MenuEntry CDManager::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_CD")),            _("<alt>C"), 0,        BRANCH },
    { _("_Login"),          _("<ctl>L"), LOGIN,    ITEM },
    { _("_Logout"),         _("<ctl>O"), LOGOUT,   ITEM },
    { "",                   "",          0,        SEPARATOR },
    { _("E_xit"),           _("<ctl>Q"), EXIT,     ITEM },
    { _("_Edit"),          _("<alt>E"),  MEDIT,    BRANCH },
    { _("_New"),            _("<ctl>N"), NEW,      ITEM },
    { _("_Edit"),           _("<ctl>E"), EDIT,     ITEM },
    { _("_Delete"),         _("<ctl>D"), DELETE,   ITEM }
};

/// Defaultconstructor; all widget are created
/// \param options: Options for the program
//-----------------------------------------------------------------------------
   : XApplication (PACKAGE " V" PRG_RELEASE), relMovies ("movies"),
CDManager::CDManager ()
   : XApplication (PACKAGE " V" PRG_RELEASE), cds (4, 1), records (), songs (),
     movies (4, 4) {
   Language::init ();


   // Create controls
   Glib::ustring ui ("<ui><menubar name='Menu'>"
		     "  <menu action='CD'>"
   addMenus (menuItems, sizeof (menuItems) / sizeof (*menuItems));
   showHelpMenu ();

   getClient ()->pack_start (*mgrUI->get_widget("/Menu"), Gtk::PACK_SHRINK);
   getClient ()->pack_start (nb, Gtk::PACK_EXPAND_WIDGET);

   Gtk::ScrolledWindow* scrlSongs (new Gtk::ScrolledWindow);
   Gtk::ScrolledWindow* scrlMovies (new Gtk::ScrolledWindow);

   nb.append_page (cds, _("C_Ds"), true);
   cds.show ();
   nb.append_page (movies, _("_Movies"), true);
   movies.show ();
   songs.signalChanged.connect (mem_fun (*this, &CDManager::songChanged));
   scrlSongs.add (songs);
   scrlRecords.add (records);
   scrlSongs.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlRecords.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlSongs.show ();
   scrlRecords.show ();

   mSongs = Gtk::ListStore::create (colSongs);
   mRecords = Gtk::TreeStore::create (colRecords);

   records.set_model (mRecords);
   songs.set_model (mSongs);
   records.show ();
   songs.show ();

   songs.append_column (_("Song"), colSongs.name);
   songs.append_column (_("Genre"), colSongs.duration);
   songs.get_column (0)->set_min_width (400);
   records.signalOwnerChanged.connect (mem_fun (*this, &CDManager::artistChanged));
   movies.signalObjectChanged.connect (mem_fun (*this, &CDManager::movieChanged));
   records.append_column (_("Interpret/Record"), colRecords.name);
   records.append_column (_("Year"), colRecords.year);
   records.append_column (_("Genre"), colRecords.genre);
   records.get_column (0)->set_min_width (400);
   records.get_selection ()->set_mode (Gtk::SELECTION_EXTENDED);
   records.get_selection ()->set_select_function
      (mem_fun (*this, &CDManager::canSelect));
   cds->add1 (*manage (scrlRecords));
   cds.attach (*manage (new Gtk::Label (_("Records"))), 0, 1, 0, 1,
	       Gtk::SHRINK, Gtk::SHRINK, 5, 5);
   cds.attach (*manage (new Gtk::Label (_("Songs"))), 0, 1, 2, 3,
	       Gtk::SHRINK, Gtk::SHRINK, 5, 5);
   cds.attach (scrlRecords, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND,
	       Gtk::FILL | Gtk::EXPAND, 5, 5);
   cds.attach (scrlSongs, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND,
	       Gtk::FILL | Gtk::EXPAND, 5, 5);

   apMenus[SAVE]->set_sensitive (false);

   show ();

#define TEST
#ifdef TEST
#endif
}
//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDManager::~CDManager () {
   TRACE9 ("CDManager::~CDManager ()");
}

//-----------------------------------------------------------------------------
/// Saves the DB
//-----------------------------------------------------------------------------
/// Command-handler
/// \param menu: ID of command (menu)
   writeChangedEntries ();
void CDManager::command (int menu) {
   TRACE2 ("CDManager::command (int) - " << menu);
   switch (menu) {
   case LOGIN:
      XGP::TLoginDialog<CDManager>::create (_("Database login"), *this,
					    &CDManager::login)->setCurrentUser ();
      break;

   case LOGOUT:
      Database::close ();
      enableMenus (false);
      status.pop ();
      status.push (_("Disconnected!"));
      break;
/// Saves the DB
   case NEW: {
      HRecord hRec;
      RecordEdit* dlg = new RecordEdit (hRec);
      dlg->run ();
      break; }

   case EDIT: {
      Gtk::TreeSelection::ListHandle_Path list
	 (records.get_selection ()->get_selected_rows ());
      Check3 (list.size ());
      for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
	   i != list.end (); ++i) {
	 Gtk::TreeIter iter (mRecords->get_iter (*i)); Check3 (iter);
	 TRACE9 ("CDManager::command (int) - EDIT: " << (**iter)[colRecords.name]);
	 Check1 (typeid (*((**iter)[colRecords.entry])) == typeid (HRecord));
	 YGP::IHandle* phRec ((**iter)[colRecords.entry]);
	 Check1 (typeid (*phRec) == typeid (HRecord));
	 new RecordEdit (*(HRecord*)(phRec));
      }
      break; }

   case DELETE:
      break;

   case EXIT:
      hide ();
      break;

   default:
      XApplication::command (menu);
   } // end-switch
//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CDManager::showAboutbox () {
   std::string ver (_("Copyright (C) 2004, 2005 Markus Schwab"
                      "\ne-mail: <g17m0@lycos.com>\n\nCompiled on %1 at %2"));
   std::string ver (_("Copyright (C) 2004 Markus Schwab"
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
/// Displays a dialog to Login to the database

//-----------------------------------------------------------------------------
/// Login to the database with the passed user/password pair
/// \param user: User to connect to the DB with
/// \param pwd: Password for user
/// \returns bool: True, if login could be performed
//-----------------------------------------------------------------------------
bool CDManager::login (const Glib::ustring& user, const Glib::ustring& pwd) {
   TRACE9 ("CDManager::login (const Glib::ustring&, const Glib::ustring&) - "
	   << user << '/' << pwd);

   try {
      Database::connect (DBNAME, user.c_str (), pwd.c_str ());
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't connect to database!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Login error"));
      dlg.run ();
      return false;
   }

   enableMenus (true);

   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &CDManager::loadDatabase), false));
   return true;
//-----------------------------------------------------------------------------
/// Enables or disables the menus according to the status of the program
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableMenus (bool enable) {
   apMenus[LOGOUT]->set_sensitive (enable);
   apMenus[MEDIT]->set_sensitive (enable);
   apMenus[LOGIN]->set_sensitive (!enable);
   apMenus[EXPORT]->set_sensitive (enable);
   apMenus[IMPORT_MP3]->set_sensitive (enable);

   cds.set_sensitive (enable);
   movies.set_sensitive (enable);
//-----------------------------------------------------------------------------
/// Enables or disables the edit-menus entries according to the selection
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableEdit (SELECTED selected) {
   TRACE9 ("CDManager::enableEdit (SELECTED) - " << selected);
void CDManager::enableEdit (bool enable) {
   apMenus[EDIT]->set_sensitive (enable);
   apMenus[DELETE]->set_sensitive (enable);
//-----------------------------------------------------------------------------
/// Loads the database and shows its contents.
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadDatabase () {
   TRACE9 ("CDManager::loadDatabase ()");
   status.pop ();
   status.push (_("Reading database ..."));

   if (nb.get_current_page ()) {
      loadMovies ();
   unsigned long int cRecords (0), cMovies (0);

   try {
      // Load data from record table
      Database::store ("SELECT r.id, r.name, i.name, year, g.genre FROM "
		       "Records r, Interprets i, Genres g "
		       "WHERE r.interpret = i.id AND r.genre = g.id "
		       "ORDER BY i.name, r.name");
      TRACE8 ("CDManager::loadDatabase () - Found " << Database::resultSize ()
	      << " records");

      if (cRecords = Database::resultSize ()) {
	 Glib::ustring artist;
	 HInterpret hArtist (new Interpret);
	 Gtk::TreeRow lastRowArtist;

	 while (Database::hasData ()) {
	    // Create new interpret, if it has changed
	    artist = Glib::locale_to_utf8 (Database::getResultColumnAsString (2));
	    if (hArtist->name != artist) {
	       lastRowArtist = *mRecords->append ();
	       hArtist.define ();
	       lastRowArtist[colRecords.name] = hArtist->name = artist;
	       lastRowArtist[colRecords.entry] = new HInterpret (hArtist);
	    }

	    // Fill and store record entry from DB-values
	    Record* newRec (new Record);
	    newRec->id = Database::getResultColumnAsUInt (0);
	    newRec->name = Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	    newRec->year = Database::getResultColumnAsUInt (3);

	    Gtk::TreeModel::Row row (*(mRecords->append (lastRowArtist.children ())));
	    newRec->name = row[colRecords.name] =
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	    if (newRec->year)
	       row[colRecords.year] = Database::getResultColumnAsString (3);

	    newRec->genre = row[colRecords.genre] = 
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (4));
	    row[colRecords.entry] = new HRecord (newRec);
	    Database::getNextResultRow ();
	 } // end-while has records
	 Check3 (mRecords->children ().begin());
	 records.get_selection ()->select (mRecords->children ().begin());
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available records!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }

   try {
      // Load data from movies table
      Database::store ("SELECT m.id, m.name, m.director, year, g.genre "
		       "FROM Movies m, Genres g WHERE m.genre = g.id");
      TRACE8 ("CDManager::loadDatabase () - Found " << Database::resultSize ()
	      << " movies");

      if (cMovies = Database::resultSize ()) {
	 while (Database::hasData ()) {
	    Database::getNextResultRow ();
	 } // end-while
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }

   Check3 ((nb.get_current_page () >= 0) || (nb.get_current_page () < 2));
   enableEdit (nb.get_current_page () ? cMovies : cRecords);
}

//-----------------------------------------------------------------------------
/// Checks if a row in the record treeview can be selected
/// \param model: Model of record treeview
/// \param path: Enty which is about to be selected
/// \returns bool: True if enty can be selected
//-----------------------------------------------------------------------------
bool CDManager::canSelect (const Glib::RefPtr<Gtk::TreeModel>& model,
			   const Gtk::TreeModel::Path& path, bool) {
  const Gtk::TreeModel::iterator iter (model->get_iter (path));
  return iter->children ().empty (); // only allow leaf nodes to be selected
}

/// Exports the stored information to HTML documents
//-----------------------------------------------------------------------------
/// \param argv: Array with pointer to parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main (int argc, char* argv[]) {
   Gtk::Main appl (argc, argv);
   CDManager win;
   appl.run (win);
   return 0;
}

