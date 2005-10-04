//$Id: CDManagerDel.cpp,v 1.4 2005/10/04 22:50:51 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.4 $
//AUTHOR      : Markus Schwab
//CREATED     : 24.1.2005
//COPYRIGHT   : Copyright (C) 2005

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

#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "CDManager.h"


//-----------------------------------------------------------------------------
/// Deletes the current selection
//-----------------------------------------------------------------------------
void CDManager::deleteSelection () {
   switch(nb.get_current_page ()) {
   case 1:
      deleteSelectedMovies ();
      break;

   case 0:
      if (records.has_focus ())
	 deleteSelectedRecords ();
      else if (songs.has_focus ())
	 deleteSelectedSongs ();
   } // end-switch
}

//-----------------------------------------------------------------------------
/// Removes the selected records or artists from the listbox. Depending objects
/// (records or songs) are deleted too.
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
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    HRecord hRecord (records.getRecordAt (child));
	    if (!hRecord->areSongsLoaded () && hRecord->getId ())
	       loadSongs (hRecord);
	    deleteRecord (child);
	 }
	 undoEntities.push_back (HEntity::cast (artist));
	 records.getModel ()->erase (iter);
	 if (artist->getId ())
	    deletedInterprets.push_back (artist);

	 std::map<HInterpret, HInterpret>::iterator iArtist (changedInterprets.find (artist));
	 if (iArtist != changedInterprets.end ())
	    changedInterprets.erase (changedInterprets.find (artist));
      }
   }
   apMenus[SAVE]->set_sensitive ();
   apMenus[UNDO]->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Deletes the passed record
/// \param record: Iterator to record to delete
//-----------------------------------------------------------------------------
void CDManager::deleteRecord (const Gtk::TreeIter& record) {
   Check2 (record->children ().empty ());

   HRecord hRec (records.getRecordAt (record));
   TRACE9 ("CDManager::deleteRecord (const Gtk::TreeIter&) - Deleting record "
	   << hRec->getName ());
   Check3 (relRecords.isRelated (hRec));
   HInterpret hArtist (relRecords.getParent (hRec)); Check3 (hArtist.isDefined ());

   // Remove related songs
   TRACE3 ("CDManager::deleteRecord (const Gtk::TreeIter&) - Remove Songs");
   while (relSongs.isRelated (hRec)) {
      std::vector<HSong>::iterator s (relSongs.getObjects (hRec).begin ());
      undoEntities.push_back (HEntity::cast (*s));
      deletedSongs[*s] = hRec;
      relSongs.unrelate (hRec, *s);
   }
   undoEntities.push_back (HEntity::cast (hRec));
   deletedRecords[hRec] = hArtist;
   relRecords.unrelate (hArtist, hRec);

   std::map<HRecord, HRecord>::iterator iRec (changedRecords.find (hRec));
   if (iRec != changedRecords.end ())
      changedRecords.erase (iRec);

   // Delete artist from listbox if it doesn't have any records
   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
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

      undoEntities.push_back (HEntity::cast (song));
      deletedSongs[song] = record;
      relSongs.unrelate (record, song);

      std::map<HSong, HSong>::iterator iSong (changedSongs.find (song));
      if (iSong != changedSongs.end ())
	 changedSongs.erase (changedSongs.find (song));
      songs.getModel ()->erase (iter);
   }

   apMenus[UNDO]->set_sensitive ();
   apMenus[SAVE]->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Removes the selected movies or directors from the listbox. Depending movies
/// are deleted too.
//-----------------------------------------------------------------------------
void CDManager::deleteSelectedMovies () {
   TRACE9 ("CDManager::deleteSelectedMovies ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (movies.get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (movies.get_model ()->get_iter (*i)); Check3 (iter);
      if ((*iter)->parent ())                 // A movie is going to be deleted
	 deleteMovie (iter);
      else {                               // A director is going to be deleted
	 TRACE9 ("CDManager::deleteSelectedMovies () - Deleting " <<
		 iter->children ().size () << " children");
	 HDirector director (movies.getDirectorAt (iter)); Check3 (director.isDefined ());
	 while (iter->children ().size ()) {
	    Gtk::TreeIter child (iter->children ().begin ());
	    deleteMovie (child);
	 }
	 movies.getModel ()->erase (iter);

	 undoEntities.push_back (HEntity::cast (director));
	 // Though director is already removed from the listbox, it still
	 // has to be removed from the database
	 if (director->getId ())
	    deletedDirectors.push_back (director);

	 std::map<HDirector, HDirector>::iterator iDirector (changedDirectors.find (director));
	 if (iDirector != changedDirectors.end ())
	    changedDirectors.erase (changedDirectors.find (director));
      }
   }
   apMenus[UNDO]->set_sensitive ();
   apMenus[SAVE]->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Deletes the passed movie
/// \param movie: Iterator to movie to delete
//-----------------------------------------------------------------------------
void CDManager::deleteMovie (const Gtk::TreeIter& movie) {
   Check2 (movie->children ().empty ());

   HMovie hMovie (movies.getMovieAt (movie));
   TRACE9 ("CDManager::deleteMovie (const Gtk::TreeIter&) - Deleting movie "
	   << hMovie->getName ());
   Check3 (relMovies.isRelated (hMovie));
   HDirector hDirector (relMovies.getParent (hMovie)); Check3 (hDirector.isDefined ());

   undoEntities.push_back (HEntity::cast (hMovie));
   deletedMovies[hMovie] = hDirector;
   relMovies.unrelate (hDirector, hMovie);

   std::map<HMovie, HMovie>::iterator iMovie (changedMovies.find (hMovie));
   if (iMovie != changedMovies.end ())
      changedMovies.erase (changedMovies.find (hMovie));

   Glib::RefPtr<Gtk::TreeStore> model (movies.getModel ());
   model->erase (movie);
}
