//$Id

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 4.10.2005
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

#define CHECK 3
#define TRACELEVEL 1
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "CDManager.h"

//-----------------------------------------------------------------------------
/// Undoes the changes to a movie
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoMovie (const HEntity& obj) {
   HMovie movie (HMovie::cast (obj));
   TRACE1 ("CDManager::undoMovie (const HEntity&) - Undoing " << movie->getId () << '/' << movie->getName ());

   std::map<HMovie, HMovie>::iterator i (changedMovies.find (movie));
   if (i != changedMovies.end ()) {
      TRACE1 ("CDManager::undoMovie (const HEntity&) - Undo to " << i->second->getId () << '/' << i->second->getName ());
      Gtk::TreeRow row (*movies.getObject (obj));
      *(i->first) = *(i->second);
      movies.update (row);

      changedMovies.erase (i);
   }
   else {
      Check3 (deletedMovies.size ());
      std::map<HMovie, HDirector>::iterator i (deletedMovies.find (movie));
      Check2 (i != deletedMovies.end ());

      HMovie movie (i->first);
      Check3 (movie.isDefined ());
      Check3 (i->second.isDefined ());
      Check3 (movies.getOwner (i->second));

      movies.append (movie, *movies.getOwner (i->second));
      relMovies.relate (i->second, i->first);
      deletedMovies.erase (i);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a record
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoRecord (const HEntity& obj) {
   HRecord record (HRecord::cast (obj));
   TRACE1 ("CDManager::undoRecord (const HEntity&) - Undoing " << record->getId () << '/' << record->getName ());

      std::map<HRecord, HRecord>::iterator i (changedRecords.find (record));
      if (i != changedRecords.end ()) {
	 TRACE1 ("CDManager::undoRecord (const HEntity&) - Undo to " << i->second->getId () << '/' << i->second->getName ());
	 Gtk::TreeRow row (*records.getObject (obj));
	 *(i->first) = *(i->second);
	 records.update (row);

	 changedRecords.erase (i);
      }
      else {
	 Check3 (deletedRecords.size ());
	 std::map<HRecord, HInterpret>::iterator i (deletedRecords.find (record));
	 Check2 (i != deletedRecords.end ());

	 HRecord record (i->first);
	 Check3 (record.isDefined ());
	 Check3 (i->second.isDefined ());
	 Check3 (records.getOwner (i->second));

	 records.append (record, *records.getOwner (i->second));
	 relRecords.relate (i->second, i->first);
	 deletedRecords.erase (i);
      }
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a song
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoSong (const HEntity& obj) {
   HSong song (HSong::cast (obj));
   TRACE1 ("CDManager::undoSong (const HEntity&) - Undo " << song->getId () << '/' << song->getName ());

   std::map<HSong, HSong>::iterator i (changedSongs.find (song));
   if (i != changedSongs.end ()) {
      TRACE1 ("CDManager::undoSong (const HEntity&) - Undo to " << i->second->getId () << '/' << i->second->getName ());
      Check3 (relSongs.getParent (i->second));
      Gtk::TreeIter rec (records.getObject (HEntity::cast (relSongs.getParent (i->second))));

      *(i->first) = *(i->second);
      Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ()); Check3 (sel);
      if (sel->get_selected_rows ().size ()
	  && (records.getModel ()->get_iter (*sel->get_selected_rows ().begin ()) == rec)) {
	 Gtk::TreeRow row (*songs.getSong (i->first));
	 songs.update (row);
      }
      changedSongs.erase (i);
   }
   else {
      Check3 (deletedSongs.size ());
      std::map<HSong, HRecord>::iterator i (deletedSongs.find (song));
      Check2 (i != deletedSongs.end ());

      Check3 (i->first.isDefined ());
      Check3 (i->second.isDefined ());
      Check3 (records.getObject (HEntity::cast (i->second)));
      relSongs.relate (i->second, i->first);

      Gtk::TreeIter record (records.getObject (HEntity::cast (i->second))); Check3 (record);
      Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ()); Check3 (sel);
      if (sel->get_selected_rows ().size ()
	  && (records.getModel ()->get_iter (*sel->get_selected_rows ().begin ()) == record))
	 songs.append (song);

      deletedSongs.erase (i);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a celebrity
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoCelebrity (const HEntity& obj) {
   HCelebrity celeb (HCelebrity::cast (obj));
   std::map<HDirector, HDirector>::iterator i (changedDirectors.find (celeb));
   if (i != changedDirectors.end ()) {
      TRACE1 ("CDManager::undo () - Undo to director " << i->second->getId () << '/' << i->second->getName ());
      Gtk::TreeRow row (*movies.getObject (obj));
      *(i->first) = *(i->second);
      movies.update (row);

      changedDirectors.erase (i);
   }
   else {
      std::vector<HDirector>::iterator i (std::find (deletedDirectors.begin (), deletedDirectors.end (), celeb));
      if (i != deletedDirectors.end ()) {
	 Check3 (i->isDefined ());
	 movies.append (*i);
	 deletedDirectors.erase (i);
      }
      else {
	 std::map<HInterpret, HInterpret>::iterator i (changedInterprets.find (celeb));
	 if (i != changedInterprets.end ()) {
	    TRACE1 ("CDManager::undo () - Undo to interpret " << i->second->getId () << '/' << i->second->getName ());
	    Gtk::TreeRow row (*movies.getObject (obj));
	    *(i->first) = *(i->second);
	    records.update (row);

	    changedInterprets.erase (i);
	 }
	 else {
	    std::vector<HInterpret>::iterator i (std::find (deletedInterprets.begin (), deletedInterprets.end (), celeb));
	    Check3 (i != deletedInterprets.end ());
	    Check3 (i->isDefined ());
	    records.append (*i);
	    deletedInterprets.erase (i);
	 }
      }
   }
}

//-----------------------------------------------------------------------------
/// Undos changes
//-----------------------------------------------------------------------------
void CDManager::undo () {
   TRACE9 ("CDManager::undo () - " << undoEntities.size ());
   Check2 (undoEntities.size ());

   HEntity obj (undoEntities.back ()); Check3 (obj.isDefined ());
   undoEntities.pop_back ();
   if (undoEntities.empty ())
      apMenus[UNDO]->set_sensitive (false);

   TRACE1 ("CDManager::undo () - Type: " << typeid (*obj).name ());
   if (typeid (*obj) ==  typeid (Movie))
      undoMovie (obj);
   else if (typeid (*obj) == typeid (Record))
      undoRecord (obj);
   else if (typeid (*obj) == typeid (Song))
      undoSong (obj);
   else if (typeid (*obj) == typeid (Celebrity))
      undoCelebrity (obj);
   else
      Check1 (0);

}
