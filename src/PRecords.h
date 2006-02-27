#ifndef PRECORDS_H
#define PRECORDS_H

//$Id: PRecords.h,v 1.10 2006/02/27 20:44:35 markus Rel $

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



#include <vector>

#include <YGP/Relation.h>

#include "Song.h"
#include "Record.h"
#include "SongList.h"
#include "Interpret.h"
#include "RecordList.h"

#include "NBPage.h"


// Forward declarations
class Genres;
class LanguageImg;


/**Class handling the records/songs notebook-page
 */
class PRecords : public NBPage {
 public:
   PRecords (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave, const Genres& genres);
   virtual ~PRecords ();

   virtual void loadData ();
   virtual void saveData () throw (Glib::ustring);
   virtual void getFocus ();
   virtual void addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction);
   virtual void deleteSelection ();
   virtual void undo ();
   virtual void clear ();

   virtual void export2HTML (unsigned int fd, const std::string& lang);
   void addEntry (const Glib::ustring&artist, const Glib::ustring& record,
		  const Glib::ustring& song, unsigned int track);

 private:
   PRecords ();

   PRecords (const PRecords& other);
   const PRecords& operator= (const PRecords& other);

   void interpretChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);
   void recordChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);
   void songChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);

   Gtk::TreeIter addInterpret (const HInterpret& interpret);
   Gtk::TreeIter addRecord (Gtk::TreeIter& parent, HRecord& record);
   Gtk::TreeIter addSong (HSong& song);

   void newInterpret ();
   void newRecord ();
   void newSong ();

   void songSelected ();
   void recordSelected ();
   void deleteRecord (const Gtk::TreeIter& record);
   void deleteSelectedRecords ();
   void deleteSelectedSongs ();
   void deleteSong (const HSong& song, const HRecord& record);

   void undoSong (const Undo& last);
   void undoRecord (const Undo& last);
   void undoInterpret (const Undo& last);

   void loadSongs (const HRecord& record);

   enum { INTERPRET, RECORD, SONG };

   RecordList records;                              // GUI-element holding records
   SongList   songs;

   // Model
   std::vector<HInterpret>               interprets;
   YGP::Relation1_N<HInterpret, HRecord> relRecords;
   YGP::Relation1_N<HRecord, HSong>      relSongs;
};

#endif
