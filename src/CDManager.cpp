//$Id: CDManager.cpp,v 1.24 2004/11/27 04:20:11 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//BUGS        :
//REVISION    : $Revision: 1.24 $
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

#include <sstream>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Process.h>
#include <YGP/Tokenize.h>
#include <XGP/XAbout.h>
#include <XGP/XFileDlg.h>
#include <XGP/LoginDlg.h>
#include "CDAppl.h"
#include "Settings.h"
#include "DB.h"
#include "Song.h"
#include "Movie.h"
#include "Record.h"
#include "Director.h"
#include "Interpret.h"


// Defines

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define storeObject(store, type, obj) \
   if (store.find (obj) == store.end ()) {\
      store[obj] = new type (*obj);\
      apMenus[SAVE]->set_sensitive (true);\
   }

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define defineChangeEntity(type, entity, store) \
void CDManager::entity##Changed (const HEntity& entity) {\
   H##type obj (H##type::cast (entity));\
   TRACE9 ("CDManager::" #entity "Changed (const HEntity&) - "\
	   << (obj.isDefined () ? obj->getId () : -1UL) << '/'\
	   << (obj.isDefined () ? obj->getName ().c_str () : "Undefined"));\
	   << (obj.isDefined () ? obj->id : -1UL) << '/'\
	   << (obj.isDefined () ? obj->name.c_str () : "Undefined"));\
   storeObject (store, type, obj);\
}

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define defineChangeObject(type, entity, store) \
void CDManager::entity##Changed (const H##type& entity) {\
   TRACE9 ("CDManager::" #entity "Changed (const H" #type "&) - "\
	   << (entity.isDefined () ? entity->getId () : -1UL) << '/'\
	   << (entity.isDefined () ? entity->getName ().c_str () : "Undefined"));\
	   << (entity.isDefined () ? entity->id : -1UL) << '/'\
	   << (entity.isDefined () ? entity->name.c_str () : "Undefined"));\
   storeObject (store, type, entity);\
}

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
// With a very ugly trick initialize I18n before the first use of gettext)
XGP::XApplication::MenuEntry CDManager::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_CD")),            _("<alt>C"),       0,            BRANCH },
    { _("_Login"),          _("<ctl>L"),       LOGIN,        ITEM },
    { _("_Logout"),         _("<ctl>O"),       LOGOUT,       ITEM },
    { _("_Save DB"),        _("<ctl>S"),       SAVE,         ITEM },
    { "",                   "",                0  ,          SEPARATOR },
    { _("E_xit"),           _("<ctl>Q"),       EXIT,         ITEM },
    { _("_Edit"),           _("<alt>E"),       MEDIT,        BRANCH },
    { _("New _Interpret"),  _("<ctl>N"),       NEW_ARTIST,   ITEM },
    { _("_New Record"),     _("<ctl><alt>N"),  NEW_RECORD,   ITEM },
    { _("New _Song"),       _("<ctl><shft>N"), NEW_SONG,     ITEM },
    { _("New _Director") ,  _("<ctl>N"),       NEW_DIRECTOR, ITEM },
    { _("_New Movie") ,     _("<ctl><alt>N"),  NEW_MOVIE,    ITEM },
    { "",                   "",                0  ,          SEPARATOR },
    { _("_Delete"),         _("<ctl>Delete"),  DELETE,       ITEM }
};

/// Defaultconstructor; all widget are created
/// \param options: Options for the program
//-----------------------------------------------------------------------------
   : XApplication (PACKAGE " V" PRG_RELEASE), relMovies ("movies"),
CDManager::CDManager ()
     movies (movieGenres), records (recGenres), loadedPages (-1U),
     relRecords ("records"), relSongs ("songs"), songs (genres), movies (mgenres),
     records (genres), loadedPages (-1U) {
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
   Gtk::ScrolledWindow* scrlRecords (new Gtk::ScrolledWindow);

   scrlSongs->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlMovies->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlRecords->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlSongs->add (songs);
   scrlMovies->add (movies);
   scrlRecords->add (records);
   scrlSongs->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlMovies->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlRecords->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   TRACE9 ("CDManager::CDManager () - Add NB");
   Gtk::HPaned* cds (new Gtk::HPaned);

   nb.append_page (*manage (cds), _("_Records"), true);
   nb.append_page (*manage (scrlMovies), _("_Movies"), true);
   nb.signal_switch_page ().connect (mem_fun (*this, &CDManager::pageSwitched));

   songs.get_selection ()->set_mode (Gtk::SELECTION_EXTENDED);
   songs.signalChanged.connect (mem_fun (*this, &CDManager::songChanged));
   records.signalOwnerChanged.connect (mem_fun (*this, &CDManager::artistChanged));
   records.signalObjectChanged.connect (mem_fun (*this, &CDManager::recordChanged));
   records.signalObjectGenreChanged.connect
      (mem_fun (*this, &CDManager::recordGenreChanged));

   movies.signalOwnerChanged.connect (mem_fun (*this, &CDManager::directorChanged));
   movies.signalObjectChanged.connect (mem_fun (*this, &CDManager::movieChanged));

   Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ());
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   recordSel->set_mode (Gtk::SELECTION_EXTENDED);
   recordSel->signal_changed ().connect
      (mem_fun (*this, &CDManager::recordSelected));
   cds->add1 (*manage (scrlRecords));
   cds->add2 (*manage (scrlSongs));

   status.push (_("Connect to a database ..."));

   apMenus[SAVE]->set_sensitive (false);

   show ();

#define TEST
#ifdef TEST
   login ("cdmgr", "");
#else
   XGP::TLoginDialog<CDManager>::create (_("Database login"), *this,
					 &CDManager::login)->setCurrentUser ();
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
   case SAVE:
      writeChangedEntries ();
      removeDeletedEntries ();
      apMenus[SAVE]->set_sensitive (false);
      break;

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
   case NEW_ARTIST: {
      Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
      HInterpret artist;
      artist.define ();
      recordSel->unselect_all ();
      Gtk::TreeModel::iterator i (records.append (artist));
      recordSel->select (i);
      records.scroll_to_row (records.getModel ()->get_path (i), 0.5);
      artistChanged (artist);
      break; }

   case NEW_RECORD: {
      Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
      Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
      Check3 (list.size ());
      Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
      Gtk::TreeIter p (model->get_iter (*list.begin ())); Check3 (p);
      if ((*p)->parent ())
	 p = ((*p)->parent ());

      HRecord record;
      record.define ();
      recordSel->unselect_all ();
      Gtk::TreeIter i (records.append (record, *p));
      records.expand_row (model->get_path (p), false);
      recordSel->select (i);
      records.scroll_to_row (records.getModel ()->get_path (i), 0.99);
      recordChanged (HEntity::cast (record));

      HInterpret artist;
      artist = records.getInterpretAt (p);
      relRecords.relate (artist, record);
      break; }

   case NEW_SONG: {
      Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
      Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeIter p (records.getModel ()->get_iter (*list.begin ())); Check3 (p);

      HRecord record (records.getRecordAt (p)); Check3 (record.isDefined ());
      HSong song;
      song.define ();
      relSongs.relate (record, song);
      p = songs.append (song);
      songs.scroll_to_row (songs.getModel ()->get_path (p), 0.99);
      songs.get_selection ()->select (p);
      songChanged (song);
      break; }

   case NEW_DIRECTOR: {
      Glib::RefPtr<Gtk::TreeSelection> movieSel (movies.get_selection ());
      HDirector director;
      director.define ();
      movieSel->unselect_all ();
      Gtk::TreeModel::iterator i (movies.append (director));
      movieSel->select (i);
      movies.scroll_to_row (movies.getModel ()->get_path (i), 0.5);
      // moviesChanged (director);
      break; }

   case NEW_MOVIE: {
      Glib::RefPtr<Gtk::TreeSelection> movieSel (movies.get_selection ());
      Gtk::TreeSelection::ListHandle_Path list (movieSel->get_selected_rows ());
      Check3 (list.size ());
      Glib::RefPtr<Gtk::TreeStore> model (movies.getModel ());
      Gtk::TreeIter p (model->get_iter (*list.begin ())); Check3 (p);
      if ((*p)->parent ())
	 p = ((*p)->parent ());

      HMovie movie;
      movie.define ();
      movieSel->unselect_all ();
      Gtk::TreeIter i (movies.append (movie, *p));
      movies.expand_row (model->get_path (p), false);
      movieSel->select (i);
      movies.scroll_to_row (movies.getModel ()->get_path (i), 0.99);

      HDirector director;
      director = movies.getDirectorAt (p);
      relMovies.relate (director, movie);
      break; }

   case DELETE:
      if (records.has_focus ())
	 deleteSelectedRecords ();
      else if (songs.has_focus ())
	 deleteSelectedSongs ();
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

   loadedPages = 0;
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &CDManager::loadDatabase), false));

   try {
      if (genres.empty ()) {
	 Database::store ("SELECT id, genre FROM Genres");

	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    genres[Database::getResultColumnAsUInt (0)] =
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	    Database::getNextResultRow ();
	 }
	 records.updateGenres ();
	 songs.updateGenres ();
      }

      if (mgenres.empty ()) {
	 Database::store ("SELECT id, genre FROM MGenres");

	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    mgenres[Database::getResultColumnAsUInt (0)] =
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	    Database::getNextResultRow ();
	 }
	 movies.updateGenres ();
      }

      if (Interpret::ignore.empty ()) {
	 Database::store ("SELECT word FROM Words");

	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    Celebrity::ignore.push_back
	       (Glib::locale_to_utf8 (Database::getResultColumnAsString (0)));
	    Database::getNextResultRow ();
	 }
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query needed information!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
   return true;
//-----------------------------------------------------------------------------
/// Enables or disables the menus according to the status of the program
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableMenus (bool enable) {
   apMenus[LOGOUT]->set_sensitive (enable);
   apMenus[MEDIT]->set_sensitive (enable);
   apMenus[EXPORT]->set_sensitive (enable);
   apMenus[IMPORT_MP3]->set_sensitive (enable);

   apMenus[LOGIN]->set_sensitive (enable = !enable);
   apMenus[SAVE]->set_sensitive (enable);
}

//-----------------------------------------------------------------------------
/// Enables or disables the edit-menus entries according to the selection
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableEdit (SELECTED selected) {
   TRACE9 ("CDManager::enableEdit (SELECTED) - " << selected);
   Check2 (apMenus[NEW1]); Check2 (apMenus[NEW2]);

   apMenus[NEW1]->set_sensitive (true);
   apMenus[NEW2]->set_sensitive (selected > NONE_SELECTED);
   apMenus[NEW_ARTIST]->set_sensitive (true);
   apMenus[NEW_RECORD]->set_sensitive (selected > NONE_SELECTED);
   apMenus[NEW_SONG]->set_sensitive (selected == RECORD_SELECTED);
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
   if (nb.get_current_page ())
   }
   else
   }

   enableEdit (NONE_SELECTED);
//-----------------------------------------------------------------------------
/// Callback after selecting a record
/// \param row: Selected row
/// Loads the records from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadRecords () {
   TRACE9 ("CDManager::loadRecords ()");
   try {
      records.clear ();

      std::vector<HInterpret> artists;
      unsigned long int cRecords (0);
      Database::store ("SELECT i.id, c.name, c.born, c.died FROM Interprets i,"
		       " Celebrities c WHERE c.id = i.id");

      HInterpret hArtist;
      while (Database::hasData ()) {
	 TRACE5 ("CDManager::laodDatabase () - Adding Artist "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 // Fill and store artist entry from DB-values
	 hArtist.define ();
	 hArtist->id = Database::getResultColumnAsUInt (0);
	 hArtist->name = Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	 std::string tmp (Database::getResultColumnAsString (2));
	 if (tmp != "0000")
	    hArtist->born = tmp;
	 tmp = Database::getResultColumnAsString (3);
	 if (tmp != "0000")
	    hArtist->died = tmp;
	 artists.push_back (hArtist);

	 Database::getNextResultRow ();
      }
      std::sort (artists.begin (), artists.end (), &Interpret::compByName);

      Database::store ("SELECT id, name, interpret, year, genre FROM "
		       "Records ORDER BY interpret, year");
      TRACE8 ("CDManager::loadDatabase () - Records: " << Database::resultSize ());

      if (cRecords = Database::resultSize ()) {
	 std::map<unsigned int, std::vector<HRecord> > aRecords;

	 HRecord newRec;
	 while (Database::hasData ()) {
	    // Fill and store record entry from DB-values
	    TRACE8 ("CDManager::loadDatabase () - Adding record "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	    newRec.define ();
	    newRec->id = Database::getResultColumnAsUInt (0);
	    newRec->name =
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	    newRec->year = Database::getResultColumnAsUInt (3);
	    newRec->genre = Database::getResultColumnAsUInt (4);
	    aRecords[Database::getResultColumnAsUInt (2)].push_back (newRec);
	    Database::getNextResultRow ();
	 } // end-while has records

	 for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	      i != artists.end (); ++i) {
	    Gtk::TreeModel::Row artist (records.append (*i));

	    std::map<unsigned int, std::vector<HRecord> >::iterator iRec
	       (aRecords.find ((*i)->id));
	    if (iRec != aRecords.end ()) {
	       for (std::vector<HRecord>::iterator r (iRec->second.begin ());
		    r != iRec->second.end (); ++r) {
		  records.append (*r, artist);
		  relRecords.relate (*i, *r);
	       } // end-for all records for an artist
	       aRecords.erase (iRec);
	    } // end-if artist has record
	 } // end-for all artists
	 records.expand_all ();
      } // end-if database contains records

      loadedPages |= 1;

      records.grab_focus ();

      Glib::ustring msg (_("%1 %2"));
      Glib::ustring tmp (_("Loaded %1 records"));
      tmp.replace (tmp.find ("%1"), 2, YGP::ANumeric::toString (cRecords));
      msg.replace (msg.find ("%1"), 2, tmp);

      tmp = _("(from %1 artist(s))");
      tmp.replace (tmp.find ("%1"), 2, YGP::ANumeric::toString (artists.size ()));
      msg.replace (msg.find ("%2"), 2, tmp);
      status.pop ();
      status.push (msg);
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available records!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the movies from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void CDManager::loadMovies () {
   TRACE9 ("CDManager::loadMovies ()");
   try {
      movies.clear ();
      std::vector<HDirector> directors;
      unsigned int cMovies (0);

      // Load data from movies table
      Database::store ("SELECT d.id, c.name, c.born, c.died FROM Directors d, "
		       "Celebrities c WHERE c.id = d.id");

      HDirector hDirector;
      while (Database::hasData ()) {
	 TRACE5 ("CDManager::laodDatabase () - Adding Director "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 // Fill and store artist entry from DB-values
	 hDirector.define ();
	 hDirector->id = Database::getResultColumnAsUInt (0);
	 hDirector->name = Glib::locale_to_utf8 (Database::getResultColumnAsString (1));

	 std::string tmp (Database::getResultColumnAsString (2));
	 if (tmp != "0000")
	    hDirector->born = tmp;
	 tmp = Database::getResultColumnAsString (3);
	 if (tmp != "0000")
	    hDirector->died = tmp;
	 directors.push_back (hDirector);

	 Database::getNextResultRow ();
      }
      std::sort (directors.begin (), directors.end (), &Director::compByName);

      Database::store ("SELECT id, name, director, year, genre FROM Movies "
		       "ORDER BY director, year");
      TRACE8 ("CDManager::loadDatabase () - Found " << Database::resultSize ()
	      << " movies");

      if (cMovies = Database::resultSize ()) {
	 std::map<unsigned int, std::vector<HMovie> > aMovies;

	 HMovie movie;
	 while (Database::hasData ()) {
	    // Fill and store record entry from DB-values
	    TRACE8 ("CDManager::loadDatabase () - Adding movie "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	    movie.define ();
	    movie->id = Database::getResultColumnAsUInt (0);
	    movie->name =
	       Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	    movie->year = Database::getResultColumnAsUInt (3);
	    movie->genre = Database::getResultColumnAsUInt (4);
	    aMovies[Database::getResultColumnAsUInt (2)].push_back (movie);
	    Database::getNextResultRow ();
	 } // end-while has movies

	 for (std::vector<HDirector>::const_iterator i (directors.begin ());
	      i != directors.end (); ++i) {
	    Gtk::TreeModel::Row director (movies.append (*i));

	    std::map<unsigned int, std::vector<HMovie> >::iterator iMovie
	       (aMovies.find ((*i)->id));
	    if (iMovie != aMovies.end ()) {
	       for (std::vector<HMovie>::iterator m (iMovie->second.begin ());
		    m != iMovie->second.end (); ++m) {
		  movies.append (*m, director);
		  relMovies.relate (*i, *m);
	       } // end-for all records for an artist
	       aMovies.erase (iMovie);
	    } // end-if director has movies
	 } // end-for all directors
	 records.expand_all ();
      } // end-if database contains records

      movies.expand_all ();

      movies.grab_focus ();

      Glib::ustring msg (_("Loaded %1 movies"));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (cMovies));
      status.pop ();
      status.push (msg);

      loadedPages |= 2;
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query available movies!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CDManager::recordSelected () {
   TRACE9 ("CDManager::recordSelected ()");
   songs.clear ();
   Check3 (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list
      (records.get_selection ()->get_selected_rows ());
   TRACE9 ("CDManager::recordSelected () - Size: " << list.size ());
   if (list.size ()) {
      Gtk::TreeIter i (records.get_model ()->get_iter (*list.begin ())); Check3 (i);

      if ((*i)->parent ()) {
	 HRecord hRecord (records.getRecordAt (i)); Check3 (hRecord.isDefined ());
	 if (!hRecord->areSongsLoaded () && hRecord->getId ())
	    loadSongs (hRecord);
	 if (!hRecord->songsLoaded)
	 // Add related songs to the listbox
	 if (relSongs.isRelated (hRecord))
	    for (std::vector<HSong>::iterator i (relSongs.getObjects (hRecord).begin ());
		 i != relSongs.getObjects (hRecord).end (); ++i)
	       songs.append (*i);

	 enableEdit (OBJECT_SELECTED);
      }
	 enableEdit (RECORD_SELECTED);
	 enableEdit (OWNER_SELECTED);
   }
	 enableEdit (ARTIST_SELECTED);
      enableEdit (NONE_SELECTED);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a movie
/// \param row: Selected row
/// Loads the songs for the passed record
/// \param record: Handle to the record for which to load songs
//-----------------------------------------------------------------------------
void CDManager::loadSongs (const HRecord& record) {
   TRACE9 ("CDManager::loadSongs (const HRecord& record) - "
	   << (record.isDefined () ? record->name.c_str () : "Undefined"));
   Check1 (record.isDefined ());

   try {
      std::stringstream query;
      query << "SELECT id, name, duration, genre, track FROM Songs WHERE"
	 " idRecord=" << record->id;
      Database::store (query.str ().c_str ());

      HSong song;
      while (Database::hasData ()) {
	 song.define ();
	 song->id = Database::getResultColumnAsUInt (0);
	 song->name = Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
	 std::string time (Database::getResultColumnAsString (2));
	 if (time != "00:00:00")
	    song->duration = time;
	 song->genre = Database::getResultColumnAsUInt (3);
	 unsigned int track (Database::getResultColumnAsUInt (4));
	 if (track)
	    song->track = track;

	 relSongs.relate (record, song);
	 Database::getNextResultRow ();
      } // end-while

      record->songsLoaded = true;
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the songs for record %1!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, record->name);
      msg.replace (msg.find ("%2"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// \param song: Handle to changed song
//-----------------------------------------------------------------------------
defineChangeObject(Song, song, changedSongs)

//-----------------------------------------------------------------------------
// void CDManager::artistChanged (const HInterpret& artist)
/// Callback when changing an artist
/// \param entity: Handle to changed artist
//-----------------------------------------------------------------------------
defineChangeObject(Interpret, artist, changedInterprets)

//-----------------------------------------------------------------------------
// void CDManager::recordChanged (const HRecord& record)
/// Callback when changing a record
/// \param entity: Handle to changed record
//-----------------------------------------------------------------------------
defineChangeEntity(Record, record, changedRecords)

//-----------------------------------------------------------------------------
// void CDManager::directorChanged (const HDirector& director)
/// Callback when changing the director of a movie
/// \param director: Handle to changed director
//-----------------------------------------------------------------------------
defineChangeObject(Director, director, changedDirectors)

//-----------------------------------------------------------------------------
// void CDManager::movieChanged (const HMovie& movie)
/// Callback when changing a movie
/// \param movie: Handle to changed movie
//-----------------------------------------------------------------------------
defineChangeEntity(Movie, movie, changedMovies)

//-----------------------------------------------------------------------------
/// Callback (additional to recordChanged) when the genre of a record is being
/// changed
/// \param record: Handle to changed record
//-----------------------------------------------------------------------------
void CDManager::recordGenreChanged (const HEntity& record) {
   Check1 (record.isDefined ());
   HRecord rec (HRecord::cast (record));
   TRACE9 ("CDManager::recordGenreChanged (const HInterpret& record) - "
	   << (rec.isDefined () ? rec->getId () : -1UL) << '/'
	   << (rec.isDefined () ? rec->getName ().c_str () : "Undefined"));
	   << (rec.isDefined () ? rec->id : -1UL) << '/'
	   << (rec.isDefined () ? rec->name.c_str () : "Undefined"));
      for (std::vector<HSong>::iterator i (relSongs.getObjects (rec).begin ());
	   i != relSongs.getObjects (rec).end (); ++i)
	 if (!(*i)->getGenre ()) {
	    (*i)->setGenre (rec->getGenre ());
	 if (!(*i)->genre) {
	    (*i)->genre = rec->genre;

      recordSelected ();
   }
}

//-----------------------------------------------------------------------------
/// Escapes the quotes in values for the database
/// \param value: Value to escape
/// Removes deleed entries from the database
//-----------------------------------------------------------------------------
void CDManager::removeDeletedEntries () {
   try {
      while (deletedInterprets.size ()) {
	 HInterpret artist (*deletedInterprets.begin ()); Check3 (artist.isDefined ());
	 try {
	    std::stringstream query;
	    query << "DELETE FROM Interprets WHERE id=" << artist->id;
	    Database::store (query.str ().c_str ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't delete interpret `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, artist->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }
	 deletedInterprets.erase (deletedInterprets.begin ());
      } // endwhile

      while (deletedRecords.size ()) {
	 HRecord record (*deletedRecords.begin ()); Check3 (record.isDefined ());
	 try {
	    std::stringstream query;
	    query << "DELETE FROM Records WHERE id=" << record->id;
	    Database::store (query.str ().c_str ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't delete record `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, record->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }
	 deletedRecords.erase (deletedRecords.begin ());
      } // endwhile

      while (deletedSongs.size ()) {
	 HSong song (*deletedSongs.begin ()); Check3 (song.isDefined ());
	 try {
	    std::stringstream query;
	    query << "DELETE FROM Songs WHERE id=" << song->id;
	    Database::store (query.str ().c_str ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't delete song `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, song->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }
	 deletedSongs.erase (deletedSongs.begin ());
      } // end-while
   }
   catch (Glib::ustring& msg) {
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Aktualizes changed entries in the database
//-----------------------------------------------------------------------------
void CDManager::writeChangedEntries () {
   try {
      while (changedInterprets.size ()) {
	 HInterpret artist (changedInterprets.begin ()->first);
	 Check3 (artist.isDefined ());
	 try {
	    std::stringstream query;
	    query << (artist->id ? "UPDATE Celebrities" : "INSERT into Celebrities")
		  << " SET name=\"" << artist->name << "\", born="
		  << (artist->born.isDefined () ? artist->born : YGP::AYear (0))
		  << ", died="
		  << (artist->died.isDefined () ? artist->died : YGP::AYear (0));

	    if (artist->id)
	       query << " WHERE id=" << artist->id;
	    Database::store (query.str ().c_str ());

	    if (!artist->id) {
	       artist->id = Database::getIDOfInsert ();
	       Database::store ("INSERT into Interprets set id=LAST_INSERT_ID()");
	    }
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write interpret `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, artist->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedInterprets.erase (changedInterprets.begin ());
      } // endwhile

      while (changedRecords.size ()) {
	 HRecord record (changedRecords.begin ()->first); Check3 (record.isDefined ());
	 try {
	    std::stringstream query;
	    query << (record->id ? "UPDATE Records" : "INSERT into Records")
		  << " SET name=\"" << record->name << "\", genre=" << record->genre;
	    if (record->year.isDefined ())
	       query << ", year=" << record->year;
	    if (relRecords.isRelated (record)) {
	       HInterpret artist (relRecords.getParent (record)); Check3 (artist.isDefined ());
	       query << ", interpret=" << artist->id;
	    }
	    if (record->id)
	       query << " WHERE id=" << record->id;
	    Database::store (query.str ().c_str ());

	    if (!record->id)
	       record->id = Database::getIDOfInsert ();
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write record `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, record->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedRecords.erase (changedRecords.begin ());
      } // endwhile

      while (changedSongs.size ()) {
	 HSong song (changedSongs.begin ()->first); Check3 (song.isDefined ());
	 try {
	    std::stringstream query;
	    query << (song->id ? "UPDATE Songs" : "INSERT into Songs")
		  << " SET name=\"" << song->name << "\", duration=\""
		  << song->duration << "\", genre=" << song->genre;
	    if (song->track.isDefined ())
	       query << ", track=" << song->track;
	    if (relSongs.isRelated (song)) {
	       HRecord record (relSongs.getParent (song)); Check3 (record.isDefined ());
	       query << ", idRecord=" << record->id;
	    }
	    if (song->id)
	       query << " WHERE id=" << song->id;
	    Database::store (query.str ().c_str ());

	    if (!song->id)
	       song->id = Database::getIDOfInsert ();
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write song `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, song->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedSongs.erase (changedSongs.begin ());
      } // endwhile

      while (changedDirectors.size ()) {
	 HDirector director (changedDirectors.begin ()->first);
	 Check3 (director.isDefined ());
	 try {
	    std::stringstream query;
	    query << (director->id ? "UPDATE Celebrities" : "INSERT into Celebrities")
		  << " SET name=\"" << director->name << "\", born="
		  << (director->born.isDefined () ? director->born : YGP::AYear (0))
		  << ", died="
		  << (director->died.isDefined () ? director->died : YGP::AYear (0));

	    if (director->id)
	       query << " WHERE id=" << director->id;
	    Database::store (query.str ().c_str ());

	    if (!director->id) {
	       director->id = Database::getIDOfInsert ();
	       Database::store ("INSERT into Directors set id=LAST_INSERT_ID()");
	    }
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write director `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, director->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedDirectors.erase (changedDirectors.begin ());
      } // endwhile

      while (changedMovies.size ()) {
	 HMovie movie (changedMovies.begin ()->first); Check3 (movie.isDefined ());
	 try {
	    std::stringstream query;
	    query << (movie->id ? "UPDATE Movies" : "INSERT into Movies")
		  << " SET name=\"" << movie->name << "\", genre=" << movie->genre;
	    if (movie->year.isDefined ())
	       query << ", year=" << movie->year;
	    if (relMovies.isRelated (movie)) {
	       HDirector director (relMovies.getParent (movie)); Check3 (director.isDefined ());
	       query << ", director=" << director->id;
	    }
	    if (movie->id)
	       query << " WHERE id=" << movie->id;
	    Database::store (query.str ().c_str ());

	    if (!movie->id)
	       movie->id = Database::getIDOfInsert ();
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write movie `%1'!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, movie->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    throw (msg);
	 }

	 changedMovies.erase (changedMovies.begin ());
      }
   }
   catch (Glib::ustring& msg) {
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Removes the selected records or artists from the listbox. Depending objects
// (records or songs) are deleted too.
//-----------------------------------------------------------------------------
void CDManager::deleteSelectedRecords () {
   TRACE9 ("CDManager::deleteSelectedRecords ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (records.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (records.get_model ()->get_iter (*i)); Check3 (iter);
      if ((*iter)->parent ())                // A record is going to be deleted
	 deleteRecord (iter);
      else {                                // An artist is going to be deleted
	 TRACE9 ("CDManager::deleteSelectedRecords () - Deleting " <<
		 iter->children ().size () << " children");
	 HInterpret artist (records.getInterpretAt (iter)); Check3 (artist.isDefined ());
	 if (iter->children ().size ())
	    while (iter->children ().size ()) {
	       Gtk::TreeIter child (iter->children ().begin ());
	       deleteRecord (child);
	    }
	 else
	    records.getModel ()->erase (iter);
	 // Though artist is allready removed from the listbox, it still
	 // has to be removed from the database
	 deletedInterprets.push_back (artist);
      }
   }
   apMenus[SAVE]->set_sensitive (true);
}

//-----------------------------------------------------------------------------
/// Deletes the passed record
/// \param record: Iterator to record to delete
//-----------------------------------------------------------------------------
void CDManager::deleteRecord (const Gtk::TreeIter& record) {
   Check2 (record->children ().empty ());

   HRecord hRec (records.getRecordAt (record));
   TRACE9 ("CDManager::deleteRecord (const Gtk::TreeIter&) - Deleting record "
	   << hRec->name);
   Check3 (relRecords.isRelated (hRec));
   HInterpret hArtist (relRecords.getParent (hRec)); Check3 (hArtist.isDefined ());

   // Remove related songs
   TRACE3 ("CDManager::deleteRecord (const Gtk::TreeIter&) - Remove Songs");
   while (relSongs.isRelated (hRec)) {
      std::vector<HSong>::iterator s
	 (relSongs.getObjects (hRec).begin ());
      relSongs.unrelate (hRec, *s);
      deletedSongs.push_back (*s);
   }
   relRecords.unrelate (hArtist, hRec);
   deletedRecords.push_back (hRec);

   // Delete artist from listbox if it doesn't have any records
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   if (!relRecords.isRelated (hArtist)) {
      TRACE9 ("CDManager::deleteRecord (const Gtk::TreeIter&) - Deleting artist "
	      << hArtist->name);

      Gtk::TreeIter parent ((*record)->parent ()); Check3 (parent);
      model->erase (record);
      model->erase (parent);
   }
   else
      model->erase (record);
}

//-----------------------------------------------------------------------------
/// Removes the selected songs from the listbox.
//-----------------------------------------------------------------------------
void CDManager::deleteSelectedSongs () {
   TRACE9 ("CDManager::deleteSelectedSongs ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (songs.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (songs.get_model ()->get_iter (*i)); Check3 (iter);
      HSong song (songs.getEntryAt (iter)); Check3 (song.isDefined ());
      Check3 (relSongs.isRelated (song));
      HRecord record (relSongs.getParent (song));

      relSongs.unrelate (record, song);
      deletedSongs.push_back (song);
      songs.getModel ()->erase (iter);
   }

   apMenus[SAVE]->set_sensitive (true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CDManager::pageSwitched (GtkNotebookPage*, guint iPage) {
   TRACE9 ("CDManager::pageSwitched (GtkNotebookPage*, guint) - " << iPage);
   Check1 (iPage < 2);

   static Gtk::UIManager::ui_merge_id idMrg (-1U);
   if (idMrg != -1U)

      apMenus[NEW_DIRECTOR]->show ();
      apMenus[NEW_MOVIE]->show ();
					   _("New _Director")),
      apMenus[NEW_ARTIST]->hide ();
      apMenus[NEW_RECORD]->hide ();
      apMenus[NEW_SONG]->hide ();
      ui += ("<menuitem action='NInterpret'/><menuitem action='NRecord'/>"
	     "<menuitem action='NSong'/>");
      apMenus[NEW_DIRECTOR]->hide ();
      apMenus[NEW_MOVIE]->hide ();
   idMrg = mgrUI->add_ui_from_string (ui);
      apMenus[NEW_ARTIST]->show ();
      apMenus[NEW_RECORD]->show ();
      apMenus[NEW_SONG]->show ();

      loadDatabase ();
}

//-----------------------------------------------------------------------------
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
