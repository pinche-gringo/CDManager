//$Id: CDManager.cpp,v 1.14 2004/11/07 02:33:58 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        : Free handles in record listbox
//BUGS        :
//REVISION    : $Revision: 1.14 $
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

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Process.h>
#include <YGP/Tokenize.h>
#include <XGP/XFileDlg.h>
#include <XGP/LoginDlg.h>
#include "CDAppl.h"
#include "Settings.h"
#include "DB.h"
#include "Song.h"
#include "Record.h"
#include "Interpret.h"


// Defines

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
      _("_CD")),            _("<alt>C"),       0,          BRANCH },
    { _("_Login"),          _("<ctl>L"),       LOGIN,      ITEM },
    { _("_Logout"),         _("<ctl>O"),       LOGOUT,     ITEM },
    { _("_Save DB"),        _("<ctl>S"),       SAVE,       ITEM },
    { "",                   "",                0  ,        SEPARATOR },
    { _("E_xit"),           _("<ctl>Q"),       EXIT,       ITEM },
    { _("_Edit"),           _("<alt>E"),       MEDIT,      BRANCH },
    { _("_New Interpret"),  _("<ctl>N"),       NEW_ARTIST, ITEM },
    { _("_New Song"),       _("<ctl><shft>N"), NEW_SONG,   ITEM },
    { _("_Delete"),         _("<del>"),        DELETE,     ITEM }
};

/// Defaultconstructor; all widget are created
/// \param options: Options for the program
//-----------------------------------------------------------------------------
   : XApplication (PACKAGE " V" PRG_RELEASE), relMovies ("movies"),
CDManager::CDManager ()
   : XApplication (PACKAGE " V" PRG_RELEASE), relRecords ("records"),
     relSongs ("songs"), movies (4, 4), records (genres), songs (genres)  {
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

   nb.append_page (cds, _("_Records"), true);
   cds.show ();
   nb.append_page (movies, _("_Movies"), true);
   movies.show ();
   songs.signalChanged.connect (mem_fun (*this, &CDManager::songChanged));
   scrlSongs.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlRecords.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlSongs.add (songs);
   scrlRecords.add (records);
   scrlSongs.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlRecords.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlSongs.show ();
   scrlRecords.show ();

   records.show ();
   songs.show ();

   records.signalOwnerChanged.connect (mem_fun (*this, &CDManager::artistChanged));
   records.signalObjectChanged.connect (mem_fun (*this, &CDManager::recordChanged));
   movies.signalObjectChanged.connect (mem_fun (*this, &CDManager::movieChanged));
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   recordSel->set_mode (Gtk::SELECTION_EXTENDED);
   recordSel->set_select_function
      (mem_fun (*this, &CDManager::canSelect));
   recordSel->signal_changed ().connect
      (mem_fun (*this, &CDManager::recordSelected));
   cds->add1 (*manage (scrlRecords));
   cds.set_position ((WIDTH - 20) >> 1);
   cds.add1 (scrlRecords);
   cds.add2 (scrlSongs);

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
      while (changedSongs.size ()) {
	 HSong song (changedSongs.begin ()->first); Check3 (song.isDefined ());
	 try {
	    std::stringstream query;
	    query << (song->id ? "UPDATE Songs" : "INSERT into Songs")
		  << " SET name=\"" << song->name << "\", track=" << song->track
		  << ", duration=\"" << song->duration << "\", genre=" << song->genre;
	    if (relSongs.isRelated (song)) {
	       HRecord record (relSongs.getParent (song)); Check3 (record.isDefined ());
	       query << ", idRecord=" << record->id;
	    }
	    if (song->id)
	       query << " WHERE id=" << song->id;
	    Database::store (query.str ().c_str ());
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't write song %1!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, song->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	    dlg.run ();
	 }

	 changedSongs.erase (changedSongs.begin ());
      } // endwhile
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
   case NEW_ARTIST:
      break;

   case NEW_SONG:
      break;

   case DELETE: {
      Glib::RefPtr<Gtk::TreeSelection> selection (records.get_selection ());
      while (selection->get_selected_rows ().size ()) {
	 Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
	 Check3 (list.size ());
	 Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

	 Gtk::TreeIter iter (records.get_model ()->get_iter (*i)); Check3 (iter);
	 HRecord hRec (records.getEntry(iter));
	 TRACE9 ("CDManager::command (int) -Deleting " << hRec->name);

	 Check3 (relRecords.isRelated (hRec));
	 HInterpret hArtist (relRecords.getParent (hRec));
	 Check3 (hArtist.isDefined ());

	 try {
	    std::stringstream id;
	    id << hRec->id;
	    std::string query ("DELETE FROM Records WHERE id=");
	    query += id.str ();
	    Database::store (query.c_str ());

	    query = "DELETE FROM Songs WHERE idRecord=";
	    query += id.str ();
	    Database::store (query.c_str ());

	    // Remove related songs
	    TRACE3 ("CDManager::command () - DELETE: Remove Songs");
	    while (relSongs.isRelated (hRec)) {
	       std::vector<HSong>::iterator s
		  (relSongs.getObjects (hRec).begin ());
	       TRACE9 ("CDManager::command () - DELETE: Remove Song "
		       << (*s)->id << '/' << (*s)->name);
	       relSongs.unrelate (hRec, *s);
	    }
	    relRecords.unrelate (hArtist, hRec);

	    // Delete artist from listbox if it doesn't have any records
	    if (!relRecords.isRelated (hArtist)) {
	       Gtk::TreeIter parent ((*iter)->parent ()); Check3 (parent);
	       records.getModel ()->erase (iter);
	       records.getModel ()->erase (parent);
	    }
	    else
	       records.getModel ()->erase (iter);
	 }
	 catch (std::exception& err) {
	    Glib::ustring msg (_("Can't delete record %1!\n\nReason: %2"));
	    msg.replace (msg.find ("%1"), 2, hRec->name);
	    msg.replace (msg.find ("%2"), 2, err.what ());
	    Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
	    dlg.run ();
	 }
	 i = list.begin ();
      } // end-while has selected entries
      break; }

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

      if (Interpret::ignore.empty ()) {
	 Database::store ("SELECT word FROM Words");

	 while (Database::hasData ()) {
	    // Fill and store artist entry from DB-values
	    Interpret::ignore.push_back
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

   cds.set_sensitive (enable);
   movies.set_sensitive (enable);
   apMenus[SAVE]->set_sensitive (enable);
}

//-----------------------------------------------------------------------------
/// Enables or disables the edit-menus entries according to the selection
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableEdit (SELECTED selected) {
   TRACE9 ("CDManager::enableEdit (SELECTED) - " << selected);
void CDManager::enableEdit (bool enable) {
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
   records.clear ();
   artists.clear ();
      loadMovies ();
   unsigned long int cRecords (0), cMovies (0);

   try {
      Database::store ("SELECT id, name, born, died FROM Interprets");

      HInterpret hArtist;
      while (Database::hasData ()) {
	 TRACE5 ("CDManager::laodDatabase () - Adding Artist "
		 << Database::getResultColumnAsUInt (0) << '/'
		 << Database::getResultColumnAsString (1));

	 // Fill and store artist entry from DB-values
	 hArtist.define ();
	 hArtist->id = Database::getResultColumnAsUInt (0);
	 hArtist->name = Glib::locale_to_utf8 (Database::getResultColumnAsString (1));
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
	    std::map<unsigned int, std::vector<HRecord> >::iterator iRec
	       (aRecords.find ((*i)->id));
	    if (iRec != aRecords.end ()) {
	       Gtk::TreeModel::Row artist (records.append (*i));

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
   if (nb.get_current_page () == 0)
      records.grab_focus ();
   enableEdit (false);
   status.pop ();
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CDManager::recordSelected () {
   TRACE9 ("CDManager::recordSelected ()");
   songs.clear ();
   Check3 (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list
   TRACE9 ("CDManager::recordSelected () - 2");

      (records.get_selection ()->get_selected_rows ());
   TRACE9 ("CDManager::recordSelected () - 3");
   TRACE9 ("CDManager::recordSelected () - Size: " << list.size ());
   if (list.size ()) {
      Gtk::TreeIter i (records.get_model ()->get_iter (*list.begin ())); Check3 (i);

      if ((*i)->parent ()) {
	 HRecord hRecord (records.getRecordAt (i)); Check3 (hRecord.isDefined ());
      HRecord hRecord (records.getEntry (i));
      if (!relSongs.isRelated (hRecord))
	 loadSongs (hRecord);

      // Add related songs to the listbox
      if (relSongs.isRelated (hRecord))
	 for (std::vector<HSong>::iterator i (relSongs.getObjects (hRecord).begin ());
	      i != relSongs.getObjects (hRecord).end (); ++i)
	    songs.append (*i);
      enableEdit (NONE_SELECTED);
  enableEdit (list.size ());
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
	 song->track = Database::getResultColumnAsUInt (4);

	 relSongs.relate (record, song);
	 Database::getNextResultRow ();
      } // end-while
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
/// Returns the interpret with the passed number
/// \param nr: Number of interpret to search for
/// \returns HInterpret: Found interpret
/// \remarks If the number specifies a non-existing interpret and undefined
///          handle is returned
//-----------------------------------------------------------------------------
HInterpret CDManager::getInterpret (unsigned int nr) const {
   TRACE9 ("CDManager::getInterpret (unsigned int) - " << nr);

   for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	i != artists.end (); ++i)
      if ((*i)->id == nr)
	 return *i;

   HInterpret tmp;
   return tmp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
defineChangeObject(Song, song, changedSongs)

void CDManager::songChanged (const HSong& song) {
   TRACE9 ("CDManager::songChanged (const HSong& song) - "
	   << (song.isDefined () ? song->id : -1UL) << '/'
	   << (song.isDefined () ? song->name.c_str () : "Undefined"));
// void CDManager::artistChanged (const HInterpret& artist)
   if (changedSongs.find (song) == changedSongs.end ()) {
      HSong newSong (new Song (*song));
      changedSongs[song] = newSong;
      apMenus[SAVE]->set_sensitive (true);
   }
}
/// Callback (additional to recordChanged) when the genre of a record is being
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
