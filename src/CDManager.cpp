//$Id: CDManager.cpp,v 1.47 2005/02/11 01:45:58 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        : - Display icon to change movie-language in statusbar
//BUGS        :
//REVISION    : $Revision: 1.47 $
//AUTHOR      : Markus Schwab
//CREATED     : 10.10.2004
//COPYRIGHT   : Copyright (C) 2004, 2005

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

#include <cerrno>
#include <cstdlib>

#include <fstream>
#include <sstream>

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/label.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Process.h>
#include <YGP/Tokenize.h>
#include <YGP/ANumeric.h>

#include <XGP/XAbout.h>
#include <XGP/XFileDlg.h>
#include <XGP/LoginDlg.h>

#include "CDAppl.h"
#include "Settings.h"
#include "Language.h"

#include "CDManager.h"


// Defines

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define storeObject(store, type, obj) \
   if (store.find (obj) == store.end ()) {\
      store[obj] = new type (*obj);\
      apMenus[SAVE]->set_sensitive ();\
   }

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define defineChangeEntity(type, entity, store) \
void CDManager::entity##Changed (const HEntity& entity) {\
   H##type obj (H##type::cast (entity));\
   TRACE9 ("CDManager::" #entity "Changed (const HEntity&) - "\
	   << (obj.isDefined () ? obj->getId () : -1UL) << '/'\
	   << (obj.isDefined () ? obj->getName ().c_str () : "Undefined"));\
   Check1 (obj.isDefined ());\
\
   storeObject (store, type, obj);\
}

// Macro to define a callback to handle changing an entity (record, movie, ...)
#define defineChangeObject(type, entity, store) \
void CDManager::entity##Changed (const H##type& entity) {\
   TRACE9 ("CDManager::" #entity "Changed (const H" #type "&) - "\
	   << (entity.isDefined () ? entity->getId () : -1UL) << '/'\
	   << (entity.isDefined () ? entity->getName ().c_str () : "Undefined"));\
   Check1 (entity.isDefined ());\
\
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
/// Defaultconstructor; all widget are created
/// \param options: Options for the program
//-----------------------------------------------------------------------------
CDManager::CDManager (Options& options)
   : XApplication (PACKAGE " V" PRG_RELEASE), relMovies ("movies"),
     relRecords ("records"), relSongs ("songs"), songs (recGenres),
     movies (movieGenres), records (recGenres), loadedPages (-1U),
     opt (options) {
   TRACE9 ("CDManager::CDManager (Options&)");

   Language::init ();

   setIconProgram (xpmProgram);
   set_default_size (WIDTH, HEIGHT);

   // Create controls
   Glib::ustring ui ("<ui><menubar name='Menu'>"
		     "  <menu action='CD'>"
		     "    <menuitem action='Login'/>"
		     "    <menuitem action='SaveDB'/>"
		     "    <menuitem action='Logout'/>"
		     "    <separator/>"
		     "    <menuitem action='Export'/>"
		     "    <menuitem action='Import'/>"
		     "    <separator/>"
		     "    <menuitem action='FQuit'/>"
		     "  </menu>"
		     "  <menu action='Edit'>"
		     "    <placeholder name='EditAction'/>"
		     "    <separator/>"
		     "    <menuitem action='Delete'/>"
		     "  </menu>"
		     "  <placeholder name='Lang'/>"
		     "  <menu action='Options'>"
		     "    <menuitem action='Prefs'/>"
		     "    <menuitem action='SavePrefs'/>"
		     "  </menu>");

   grpAction->add (Gtk::Action::create ("CD", _("_CD")));
   grpAction->add (apMenus[LOGIN] = Gtk::Action::create ("Login", _("_Login")),
		   Gtk::AccelKey ("<ctl>L"),
		   mem_fun (*this, &CDManager::showLogin));
   grpAction->add (apMenus[SAVE] = Gtk::Action::create ("SaveDB", Gtk::Stock::SAVE),
		   mem_fun (*this, &CDManager::save));
   grpAction->add (apMenus[LOGOUT] = Gtk::Action::create ("Logout", _("Log_out")),
		   Gtk::AccelKey ("<ctl>O"),
		   mem_fun (*this, &CDManager::logout));
   grpAction->add (apMenus[EXPORT] = Gtk::Action::create ("Export", _("_Export to HTML")),
		   Gtk::AccelKey ("<ctl>E"),
		   mem_fun (*this, &CDManager::exportToHTML));
   grpAction->add (apMenus[IMPORT_MP3] = Gtk::Action::create ("Import", _("_Import from MP3-info ...")),
		   Gtk::AccelKey ("<ctl>I"),
		   mem_fun (*this, &CDManager::importFromMP3));
   grpAction->add (Gtk::Action::create ("FQuit", Gtk::Stock::QUIT),
		   mem_fun (*this, &CDManager::hide));
   grpAction->add (apMenus[MEDIT] = Gtk::Action::create ("Edit", _("_Edit")));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("Delete", Gtk::Stock::DELETE,_("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &CDManager::deleteSelection));
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

   getClient ()->pack_start (*mgrUI->get_widget("/Menu"), Gtk::PACK_SHRINK);
   getClient ()->pack_start (nb, Gtk::PACK_EXPAND_WIDGET);
   getClient ()->pack_end (status, Gtk::PACK_SHRINK);

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

   TRACE9 ("CDManager::CDManager (Options&) - Add NB");
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
   sel->signal_changed ().connect (mem_fun (*this, &CDManager::recordSelected));

   sel = movies.get_selection ();
   sel->signal_changed ().connect (mem_fun (*this, &CDManager::movieSelected));

   cds->set_position ((WIDTH - 20) >> 1);
   cds->add1 (*manage (scrlRecords));
   cds->add2 (*manage (scrlSongs));

   status.push (_("Connect to a database ..."));

   apMenus[SAVE]->set_sensitive (false);

   TRACE9 ("CDManager::CDManager (Options&) - Show");
   show_all_children ();
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
void CDManager::save () {
   writeChangedEntries ();
   removeDeletedEntries ();
   apMenus[SAVE]->set_sensitive (false);
}

//-----------------------------------------------------------------------------
/// Saves the DB
//-----------------------------------------------------------------------------
void CDManager::importFromMP3 () {
   XGP::TFileDialog<CDManager>::create (_("Select file(s) to import"), *this,
					&CDManager::parseMP3Info,
					Gtk::FILE_CHOOSER_ACTION_OPEN,
					XGP::IFileDialog::MUST_EXIST
					| XGP::IFileDialog::MULTIPLE);
}

//-----------------------------------------------------------------------------
/// Edits dthe preferences
//-----------------------------------------------------------------------------
void CDManager::editPreferences () {
   Settings::create (get_window (), opt);
}

//-----------------------------------------------------------------------------
/// Adds a new interpret to the list
//-----------------------------------------------------------------------------
void CDManager::newInterpret () {
   HInterpret artist;
   artist.define ();
   addArtist (artist);
}

//-----------------------------------------------------------------------------
/// Adds a new record to the first selected interpret
//-----------------------------------------------------------------------------
void CDManager::newRecord () {
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
   Check3 (list.size ());
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   Gtk::TreeIter p (model->get_iter (*list.begin ())); Check3 (p);
   if ((*p)->parent ())
      p = ((*p)->parent ());

   HRecord record;
   record.define ();
   addRecord (p, record);
}

//-----------------------------------------------------------------------------
/// Adds a new song to the first selected record
//-----------------------------------------------------------------------------
void CDManager::newSong () {
   HSong song;
   song.define ();
   addSong (song);
}

//-----------------------------------------------------------------------------
/// Adds a new direcotor to the list
//-----------------------------------------------------------------------------
void CDManager::newDirector () {
   Glib::RefPtr<Gtk::TreeSelection> movieSel (movies.get_selection ());
   HDirector director;
   director.define ();
   directors.push_back (director);

   Gtk::TreeModel::iterator i (movies.append (director));
   movies.selectRow (i);
   recordChanged (HEntity::cast (director));
}

//-----------------------------------------------------------------------------
/// Adds a new movie to the first selected director
//-----------------------------------------------------------------------------
void CDManager::newMovie () {
   Glib::RefPtr<Gtk::TreeSelection> movieSel (movies.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (movieSel->get_selected_rows ());
   Check3 (list.size ());
   Glib::RefPtr<Gtk::TreeStore> model (movies.getModel ());
   Gtk::TreeIter p (model->get_iter (*list.begin ())); Check3 (p);
   if ((*p)->parent ())
      p = ((*p)->parent ());

   HMovie movie;
   movie.define ();
   Gtk::TreeIter i (movies.append (movie, *p));
   movies.expand_row (model->get_path (p), false);
   movies.selectRow (i);

   HDirector director;
   director = movies.getDirectorAt (p);
   relMovies.relate (director, movie);
}

//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CDManager::showAboutbox () {
   std::string ver (_("Copyright (C) 2004, 2005 Markus Schwab"
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
/// Displays a dialog to Login to the database
//-----------------------------------------------------------------------------
void CDManager::showLogin () {
   XGP::TLoginDialog<CDManager>::create (_("Database login"), *this,
					 &CDManager::login)->setCurrentUser ();
}

//-----------------------------------------------------------------------------
/// Enables or disables the menus according to the status of the program
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void CDManager::enableMenus (bool enable) {
   apMenus[LOGOUT]->set_sensitive (enable);
   apMenus[MEDIT]->set_sensitive (enable);
   apMenus[EXPORT]->set_sensitive (enable);
   apMenus[IMPORT_MP3]->set_sensitive (enable);
   apMenus[SAVE_PREFS]->set_sensitive (enable);

   nb.set_sensitive (enable);

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

   apMenus[DELETE]->set_sensitive (selected != NONE_SELECTED);
   apMenus[NEW1]->set_sensitive (true);
   apMenus[NEW2]->set_sensitive (selected > NONE_SELECTED);
   if (apMenus[NEW3])
      apMenus[NEW3]->set_sensitive (selected == OBJECT_SELECTED);
}

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
      movies.grab_focus ();
   }
   else {
      loadRecords ();
      records.grab_focus ();
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting a record
/// \param row: Selected row
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

	 // Add related songs to the listbox
	 if (relSongs.isRelated (hRecord))
	    for (std::vector<HSong>::iterator i (relSongs.getObjects (hRecord).begin ());
		 i != relSongs.getObjects (hRecord).end (); ++i)
	       songs.append (*i);

	 enableEdit (OBJECT_SELECTED);
      }
      else
	 enableEdit (OWNER_SELECTED);
   }
   else
      enableEdit (NONE_SELECTED);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a movie
/// \param row: Selected row
//-----------------------------------------------------------------------------
void CDManager::movieSelected () {
   TRACE9 ("CDManager::movieSelected ()");
   Check3 (movies.get_selection ());

   Gtk::TreeIter s (movies.get_selection ()->get_selected ());
   enableEdit (s ? OWNER_SELECTED : NONE_SELECTED);
}

//-----------------------------------------------------------------------------
// void CDManager::songChanged (const HSong& song)
/// Callback when a song is being changed
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

   if (relSongs.isRelated (rec)) {
      for (std::vector<HSong>::iterator i (relSongs.getObjects (rec).begin ());
	   i != relSongs.getObjects (rec).end (); ++i)
	 if (!(*i)->getGenre ()) {
	    (*i)->setGenre (rec->getGenre ());
	    songChanged (*i);
	 }

      recordSelected ();
   }
}

//-----------------------------------------------------------------------------
/// Escapes the quotes in values for the database
/// \param value: Value to escape
/// \returns Glib::ustring: Escaped text
//-----------------------------------------------------------------------------
Glib::ustring CDManager::escapeDBValue (const Glib::ustring& value) {
   unsigned int pos (0);
   Glib::ustring rc (value);
   while ((pos = rc.find ('"', pos)) != std::string::npos) {
      rc.replace (pos, 1, "\\\"");
      pos += 2;
   }
   TRACE9 ("CDManager::escapeDBValue (const Glib::ustring&) - Escaped: " << rc);
   return rc;
}

//-----------------------------------------------------------------------------
/// Callback when switching the notebook pages
/// \param iPage: Index of the newly selected page
//-----------------------------------------------------------------------------
void CDManager::pageSwitched (GtkNotebookPage*, guint iPage) {
   TRACE9 ("CDManager::pageSwitched (GtkNotebookPage*, guint) - " << iPage);
   Check1 (iPage < 2);

   static Gtk::UIManager::ui_merge_id idMrg (-1U);
   if (idMrg != -1U)
      mgrUI->remove_ui (idMrg);

   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <menu action='Edit'>"
		     "    <placeholder name='EditAction'>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   if (iPage) {
      if (!(loadedPages & (1 << iPage)))
	 loadDatabase ();

      ui += ("<menuitem action='NDirector'/><menuitem action='NMovie'/>"
	     "</placeholder></menu><placeholder name='Lang'><menu action='Lang'><menuitem action='Orig'/>");

      grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NDirector", Gtk::Stock::NEW,
							   _("New _Director")),
		      Gtk::AccelKey (_("<ctl>N")),
		      mem_fun (*this, &CDManager::newDirector));
      grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NMovie", _("_New Movie")),
		      Gtk::AccelKey (_("<ctl><alt>N")),
		      mem_fun (*this, &CDManager::newMovie));
      apMenus[NEW3].clear ();

      Gtk::RadioButtonGroup grpLang;
      grpAction->add (Gtk::Action::create ("Lang", _("_Language")));
      grpAction->add (Gtk::RadioAction::create (grpLang, "Orig", _("_Original name")),
		      Gtk::AccelKey ("<ctl>0"),
		      bind (mem_fun (*this, &CDManager::changeLanguage), ""));

      char accel[6];
      strcpy (accel, "<ctl>1");
      for (std::map<std::string, Language>::const_iterator i (Language::begin ());
	   i != Language::end (); ++i) {
	 TRACE9 ("CDManager::CDManager (Options&) - Adding language " << i->first);
	 ui += "<menuitem action='" + i->first + "'/>";

	 Glib::RefPtr<Gtk::RadioAction> act
	    (Gtk::RadioAction::create (grpLang, i->first, i->second.getInternational ()));
	 TRACE1 ("Langs: " << i->first << " - " << Movie::currLang);

	 if (accel[5] <= '9') {
	    grpAction->add (act, Gtk::AccelKey (accel),
			    bind (mem_fun (*this, &CDManager::changeLanguage), i->first));
	    ++accel[5];
	 }
	 else
           grpAction->add (act, bind (mem_fun (*this, &CDManager::changeLanguage), i->first));

	 if (i->first == Movie::currLang)
	    act->set_active ();
      }
      ui += "</menu></placeholder></menubar>";

      movieSelected ();
   }
   else {
      ui += ("<menuitem action='NInterpret'/><menuitem action='NRecord'/>"
	     "<menuitem action='NSong'/></placeholder></menu></menubar>");

      grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NInterpret", Gtk::Stock::NEW,
							   _("New _Interpret")),
		      Gtk::AccelKey (_("<ctl>N")),
		      mem_fun (*this, &CDManager::newInterpret));
      grpAction->add (apMenus[NEW2] = Gtk::Action::create ("NRecord", _("_New Record")),
		      Gtk::AccelKey (_("<ctl><alt>N")),
		      mem_fun (*this, &CDManager::newRecord));
      grpAction->add (apMenus[NEW3] = Gtk::Action::create ("NSong", _("New _Song")),
		      Gtk::AccelKey (_("<ctl><shft>N")),
		      mem_fun (*this, &CDManager::newSong));

      recordSelected ();
   }

   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);
}

//-----------------------------------------------------------------------------
/// Exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::exportToHTML () {
   std::string dir (opt.getDirOutput ());
   if (dir.size () && (dir[dir.size () - 1] != YGP::File::DIRSEPARATOR)) {
      dir += YGP::File::DIRSEPARATOR;
      opt.setDirOutput (dir);
   }

   try {
      exportData ();
   }
   catch (Glib::ustring& e) {
      Gtk::MessageDialog dlg (e, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the data and exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::exportData () throw (Glib::ustring) {
   TRACE9 ("CDManager::exportData ()");
   if (!(loadedPages & 1))
      loadRecords ();

   if (!(loadedPages & 2))
      loadMovies ();

   std::sort (directors.begin (), directors.end (), &Director::compByName);
   std::sort (artists.begin (), artists.end (), &Interpret::compByName);

   YGP::Tokenize langs (LANGUAGES);
   std::string lang;
   std::string tmpLang (Movie::currLang);
   std::string oldLang (getenv ("LANGUAGE"));
   while ((lang = langs.getNextNode (' ')).size ()) {
      TRACE1 ("CDManager::exportData () - Lang: " << lang);
      Glib::ustring stat (_("Exporting (language %1) ..."));
      stat.replace (stat.find ("%1"), 2, Language::findInternational (lang));
      status.push (stat);

      Glib::RefPtr<Glib::MainContext> ctx (Glib::MainContext::get_default ());
      while (ctx->iteration (false));                      // Update statusbar

      if (!loadedLangs[lang])
	 loadMovies (lang);

      setenv ("LANGUAGE", lang.c_str (), true);
      int pipes[2];
      const char* args[] = { "CDWriter", "--outputDir", opt.getDirOutput ().c_str (),
			     "--recHeader", opt.getRHeader ().c_str (),
			     "--recFooter", opt.getRFooter ().c_str (),
			     "--movieHeader", opt.getMHeader ().c_str (),
			     "--movieFooter", opt.getMFooter ().c_str (),
			     lang.c_str (), NULL };
      try {
	 pipe (pipes);
	 YGP::Process::execIOConnected ("CDWriter", args, pipes);
      }
      catch (std::string& e) {
	 Gtk::MessageDialog dlg (Glib::locale_to_utf8 (e), Gtk::MESSAGE_ERROR);
	 dlg.set_title (_("Export Error!"));
	 dlg.run ();
      }

      // Write movie-information
      Movie::currLang = lang;
      for (std::vector<HDirector>::const_iterator i (directors.begin ());
	   i != directors.end (); ++i)
	 if (relMovies.isRelated (*i)) {
	    std::stringstream output;
	    output << 'D' << **i;

	    std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	    Check3 (dirMovies.size ());
	    for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
		 m != dirMovies.end (); ++m)
	       output << "M" << **m;

	    TRACE9 ("CDManager::export () - Writing: " << output.str ());
	    ::write (pipes[1], output.str ().data (), output.str ().size ());
	 }
      Movie::currLang = tmpLang;

      // Write record-information
      for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	   i != artists.end (); ++i)
	 if (relRecords.isRelated (*i)) {
	    std::stringstream output;
	    output << 'I' << **i;

	    std::vector<HRecord>& records (relRecords.getObjects (*i));
	    Check3 (records.size ());
	    for (std::vector<HRecord>::const_iterator r (records.begin ());
		 r != records.end (); ++r)
	       output << "R" << **r;

	    TRACE9 ("CDManager::export () - Writing: " << output.str ());
	    ::write (pipes[1], output.str ().data (), output.str ().size ());
	 }
      close (pipes[1]);

      char output[128] = "";
      std::string allOut;
      int cRead;
      while ((cRead = ::read (pipes[0], output, sizeof (output))) != -1) {
	 allOut.append (output, cRead);
	 if ((unsigned int)cRead < sizeof (output))
	    break;
      }
      if (allOut.size ()) {
	 Gtk::MessageDialog dlg (Glib::locale_to_utf8 (output), Gtk::MESSAGE_INFO);
	 dlg.set_title (_("Export Warning!"));
	 dlg.run ();
      }

      close (pipes[0]);
      status.pop ();
   }
   setenv ("LANGUAGE", oldLang.c_str (), true);
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param file: Name of file to analzye
//-----------------------------------------------------------------------------
void CDManager::parseMP3Info (const std::string& file) {
   TRACE9 ("CDManager::parseMP3Info (const std::string&) - " << file);
   Check2 (file.size ());

   std::ifstream stream (file.c_str ());
   Glib::ustring artist, record, song;
   unsigned int track;
   if (stream && parseMP3Info (stream, artist, record, song, track)) {
      TRACE9 ("CDManager::parseMP3Info (const std::string&) - " << artist
	      << '/' << record << '/' << song << '/' << track);

      HInterpret group;
      Gtk::TreeIter i (records.getOwner (artist));
      if (i == records.getModel ()->children ().end ()) {
	 TRACE9 ("CDManager::parseMP3Info (const std::string&) - Adding band " << artist);
	 group.define ();
	 group->setName (artist);
	 i = addArtist (group);
      }
      else
	 group = records.getInterpretAt (i);

      HRecord rec;
      Gtk::TreeIter r (records.getObject (i, record));
      if (r == i->children ().end ()) {
	 TRACE9 ("CDManager::parseMP3Info (const std::string&) - Adding rec " << record);
	 rec.define ();
	 rec->setName (record);
	 addRecord (i, rec);
      }
      else {
	 rec = records.getRecordAt (r);
	 records.selectRow (r);
      }

      HSong hSong;
      Gtk::TreeIter s (songs.getSong (song));
      if (s == songs.getModel ()->children ().end ()) {
	 TRACE9 ("CDManager::parseMP3Info (const std::string&) - Adding song " << song);
	 hSong.define ();
	 hSong->setName (song);
	 hSong->setTrack (track);
	 addSong (hSong);
      }
      else {
	 hSong = songs.getEntryAt (s);
	 songs.scroll_to_row (songs.getModel ()->get_path (s), 0.80);
	 Glib::RefPtr<Gtk::TreeSelection> songSel (songs.get_selection ());
	 songSel->select (s);
	 if (track) {
	    hSong->setTrack (track);
	    Gtk::TreeRow row (*s);
	    songs.updateTrack (row, hSong->getTrack ());
	 }
      }
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
   TRACE9 ("CDManager::parseMP3Info (std::stream&, 3x Glib::ustring&, unsigned&) - Found: "
	   << value << "; Length: " << value.size ());
   if ((value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
      song = Glib::locale_to_utf8 (stripString (value, 3, 29));
      artist = Glib::locale_to_utf8 (stripString (value, 33, 29));
      record = Glib::locale_to_utf8 (stripString (value, 63, 29));
      if (value[0x7d] != 0x20)
	 track = value[0x7e];
      return true;
   }
   return false;
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

//-----------------------------------------------------------------------------
/// Adds an interpret to the record listbox
/// \param artist: Handle to the new interpret
/// \returns Gtk::TreeIter: Iterator to new added artist
//-----------------------------------------------------------------------------
Gtk::TreeIter CDManager::addArtist (const HInterpret& artist) {
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   artists.push_back (artist);

   Gtk::TreeModel::iterator i (records.append (artist));
   records.selectRow (i);
   artistChanged (artist);
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a record to the record listbox
/// \param parent: Iterator to the interpret of the record
/// \param record: Handle to the new record
/// \returns Gtk::TreeIter: Iterator to new added record
//-----------------------------------------------------------------------------
Gtk::TreeIter CDManager::addRecord (Gtk::TreeIter& parent, HRecord& record) {
   Gtk::TreeIter i (records.append (record, *parent));
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   records.expand_row (model->get_path (parent), false);
   records.selectRow (i);
   recordChanged (HEntity::cast (record));

   HInterpret artist;
   artist = records.getInterpretAt (parent);
   relRecords.relate (artist, record);
   return i;
}

//-----------------------------------------------------------------------------
/// Adds a song to the song listbox
/// \param song: Handle to the new song
/// \returns Gtk::TreeIter: Iterator to new added song
//-----------------------------------------------------------------------------
Gtk::TreeIter CDManager::addSong (HSong& song) {
   Glib::RefPtr<Gtk::TreeSelection> recordSel (records.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (recordSel->get_selected_rows ());
   Check3 (list.size ());
   Gtk::TreeIter p (records.getModel ()->get_iter (*list.begin ())); Check3 (p);

   HRecord record (records.getRecordAt (p)); Check3 (record.isDefined ());
   relSongs.relate (record, song);
   p = songs.append (song);
   songs.scroll_to_row (songs.getModel ()->get_path (p), 0.8);
   songs.get_selection ()->select (p);
   songChanged (song);
   return p;
}

//-----------------------------------------------------------------------------
/// Changes the language in which the movies are displayed
/// \param lang: Lanuage in which the movies should be displayed
//-----------------------------------------------------------------------------
void CDManager::changeLanguage (const std::string& lang) {
   Check3 (lang.empty () || (lang.size () == 2));

   static bool ignore (false);                  // Ignore de-selecting an entry
   ignore = !ignore;
   TRACE1 ("CDManager::changeLanguage (const std::string&) - " << lang << ": "
	   << (ignore ? "True" : "False"));
   if (ignore)
      return;

   Movie::currLang = lang;
   if ((lang.size () == 2) && !loadedLangs[lang])
      loadMovies (lang);

   movies.update (lang);
}
