#ifndef CDMANAGER_H
#define CDMANAGER_H

//$Id: CDManager.h,v 1.28 2005/01/13 22:32:57 markus Exp $

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


#include <map>
#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/paned.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/statusbar.h>

#include "Song.h"
#include "Movie.h"
#include "Genres.h"
#include "Record.h"
#include "Options.h"
#include "SongList.h"
#include "Director.h"
#include "Interpret.h"
#include "MovieList.h"
#include "RecordList.h"

#include <YGP/Relation.h>

#include <XGP/XApplication.h>


// Forward declarations
namespace YGP {
   class Entity;
}


/**Class for application to manage CDs (audio and video)
*/
class CDManager : public XGP::XApplication {
 public:
   // Manager functions
   CDManager (Options& options);
   ~CDManager ();

 private:
   // IDs for menus
   enum { LOGIN = XApplication::LAST, SAVE, LOGOUT, MEDIT, NEW_ARTIST,
	  NEW_RECORD, NEW_SONG, NEW_MOVIE, NEW_DIRECTOR, DELETE, EXPORT,
	  IMPORT_MP3, PREFERENCES, SAVE_PREF, EXIT };

   // Protected manager functions
   CDManager (const CDManager&);
   const CDManager& operator= (const CDManager&);

   // Event-handling
   virtual void command (int menu);
   virtual void showAboutbox ();
   virtual const char* getHelpfile ();
   void recordSelected ();
   void pageSwitched (GtkNotebookPage* page, guint iPage);

   typedef enum { NONE_SELECTED, ARTIST_SELECTED, RECORD_SELECTED } SELECTED;
   void enableEdit (SELECTED selected);
   bool login (const Glib::ustring& user, const Glib::ustring& pwd);
   void loadDatabase ();
   void loadRecords ();
   void loadMovies ();
   void enableMenus (bool enable);
   void loadSongs (const HRecord& record);

   Gtk::TreeIter addArtist (const HInterpret& artist);
   Gtk::TreeIter addRecord (Gtk::TreeIter& parent, HRecord& record);
   Gtk::TreeIter addSong (HSong& song);

   void artistChanged (const HInterpret& artist);
   void recordChanged (const HEntity& record);
   void recordGenreChanged (const HEntity& record);
   void songChanged (const HSong& song);
   void directorChanged (const HDirector& director);
   void movieChanged (const HEntity& movie);

   void deleteRecord (const Gtk::TreeIter& record);
   void deleteSelectedRecords ();
   void deleteSelectedSongs ();

   void removeDeletedEntries ();
   void writeChangedEntries ();
   static Glib::ustring escapeDBValue (const Glib::ustring& value);

   void exportMovies () throw (Glib::ustring);
   void exportRecords () throw (Glib::ustring);

   std::string stripString (const std::string& value, unsigned int pos, unsigned int len);
   bool parseMP3Info (std::istream& stream, Glib::ustring& artist,
		      Glib::ustring& record, Glib::ustring& song,
		      unsigned int& track);
   void parseMP3Info (const std::string& file);

   static XGP::XApplication::MenuEntry menuItems[];

   static const char* xpmProgram[];
   static const char* xpmAuthor[];

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;

   static const char* const DBNAME;

   Genres recGenres;
   Genres movieGenres;

   YGP::Relation1_N<HDirector, HMovie> relMovies;

   YGP::Relation1_N<HInterpret, HRecord> relRecords;
   YGP::Relation1_N<HRecord, HSong>      relSongs;

   Gtk::Notebook  nb;

   SongList                     songs;
   MovieList                    movies;
   RecordList                   records;

   Gtk::Statusbar status;

   std::vector<HSong>      deletedSongs;
   std::vector<HRecord>    deletedRecords;
   std::vector<HInterpret> deletedInterprets;
   std::vector<HMovie>     deletedMovies;
   std::vector<HInterpret> deletedDirectors;

   std::map<HSong,HSong>            changedSongs;
   std::map<HRecord, HRecord>       changedRecords;
   std::map<HInterpret, HInterpret> changedInterprets;
   std::map<HMovie, HMovie>         changedMovies;
   std::map<HDirector, HDirector>   changedDirectors;

   unsigned int loadedPages;

   std::vector<HDirector>  directors;
   std::vector<HInterpret> artists;

   Options& opt;
};

#endif
