//$Id: CDManagerUndo.cpp,v 1.2 2005/10/04 22:48:54 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManager
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
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
   Glib::ustring msg (_("Undone movie '%1'"));
   msg.replace (msg.find ("%1"), 2, movie->getName ());
   status.push (msg);

   Gtk::TreeRow row;
   if (i != changedMovies.end ()) {
      row = *movies.getObject (obj);
      *movie = *(i->second);
      movies.update (row);

      changedMovies.erase (i);
   }
   else {
      Check3 (deletedMovies.size ());
      std::map<HMovie, HDirector>::iterator i (deletedMovies.find (movie));
      Check2 (i != deletedMovies.end ());
      Check3 (i->second.isDefined ());
      Check3 (movies.getOwner (i->second));

      row = movies.append (movie, *movies.getOwner (i->second));
      relMovies.relate (i->second, movie);
      deletedMovies.erase (i);
   }

   Glib::RefPtr<Gtk::TreeStore> model (movies.getModel ());
   Check3 (model); Check3 (row->parent ());
   movies.expand_row (model->get_path (row->parent ()), false);
   movies.scroll_to_row (model->get_path (row), 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a record
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoRecord (const HEntity& obj) {
   HRecord record (HRecord::cast (obj));
   TRACE1 ("CDManager::undoRecord (const HEntity&) - Undoing " << record->getId () << '/' << record->getName ());

   Glib::ustring msg (_("Undone record '%1'"));
   msg.replace (msg.find ("%1"), 2, record->getName ());
   status.push (msg);

   Gtk::TreeRow row;
   std::map<HRecord, HRecord>::iterator i (changedRecords.find (record));
   if (i != changedRecords.end ()) {
      TRACE1 ("CDManager::undoRecord (const HEntity&) - Undo to " << i->second->getId () << '/' << i->second->getName ());
      row = *records.getObject (obj);
      *record = *(i->second);
      records.update (row);

      changedRecords.erase (i);
   }
   else {
      Check3 (deletedRecords.size ());
      std::map<HRecord, HInterpret>::iterator i (deletedRecords.find (record));
      Check2 (i != deletedRecords.end ());

      Check3 (i->second.isDefined ());
      Check3 (records.getOwner (i->second));

      row = records.append (record, *records.getOwner (i->second));
      relRecords.relate (i->second, record);
      deletedRecords.erase (i);
   }

   Glib::RefPtr<Gtk::TreeStore> model (records.getModel ());
   Check3 (model); Check3 (row->parent ());
   records.expand_row (model->get_path (row->parent ()), false);
   records.scroll_to_row (model->get_path (row), 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a song
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoSong (const HEntity& obj) {
   HSong song (HSong::cast (obj));
   TRACE1 ("CDManager::undoSong (const HEntity&) - Undo " << song->getId () << '/' << song->getName ());

   Glib::ustring msg (_("Undone song '%1'"));
   msg.replace (msg.find ("%1"), 2, song->getName ());
   status.push (msg);

   std::map<HSong, HSong>::iterator i (changedSongs.find (song));
   if (i != changedSongs.end ()) {
      TRACE1 ("CDManager::undoSong (const HEntity&) - Undo to " << i->second->getId () << '/' << i->second->getName ());
      Check3 (relSongs.getParent (i->second));
      Gtk::TreeIter rec (records.getObject (HEntity::cast (relSongs.getParent (i->second))));

      *song = *(i->second);
      Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ()); Check3 (sel);
      if (sel->get_selected_rows ().size ()
	  && (records.getModel ()->get_iter (*sel->get_selected_rows ().begin ()) == rec)) {
	 Gtk::TreeRow row (*songs.getSong (song));
	 songs.update (row);
	 songs.scroll_to_row (songs.getModel ()->get_path (row), 0.8);
      }
      changedSongs.erase (i);
   }
   else {
      Check3 (deletedSongs.size ());
      std::map<HSong, HRecord>::iterator i (deletedSongs.find (song));
      Check2 (i != deletedSongs.end ());

      Check3 (i->second.isDefined ());
      Check3 (records.getObject (HEntity::cast (i->second)));
      relSongs.relate (i->second, song);

      Gtk::TreeIter record (records.getObject (HEntity::cast (i->second))); Check3 (record);
      Glib::RefPtr<Gtk::TreeSelection> sel (records.get_selection ()); Check3 (sel);
      if (sel->get_selected_rows ().size ()
	  && (records.getModel ()->get_iter (*sel->get_selected_rows ().begin ()) == record))
	 songs.scroll_to_row (songs.getModel ()->get_path (songs.append (song)), 0.8);

      deletedSongs.erase (i);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the changes to a celebrity
/// \param obj: Movie to undo
//-----------------------------------------------------------------------------
void CDManager::undoCelebrity (const HEntity& obj) {
   HCelebrity celeb (HCelebrity::cast (obj));
   TRACE1 ("CDManager::undoCelebrity (const HEntity&) - Undo " << celeb->getId () << '/' << celeb->getName ());

   Glib::ustring msg (_("Undone %1 '%2'"));
   msg.replace (msg.find ("%2"), 2, celeb->getName ());
   status.push (msg);

   std::map<HDirector, HDirector>::iterator i (changedDirectors.find (celeb));
   if (i != changedDirectors.end ()) {
      Gtk::TreeRow row (*movies.getObject (obj));
      *(i->first) = *(i->second);
      movies.update (row);
      movies.scroll_to_row (movies.getModel ()->get_path (row), 0.8);

      changedDirectors.erase (i);
      msg.replace (msg.find ("%1"), 2, _("director"));
   }
   else {
      std::vector<HDirector>::iterator i (std::find (deletedDirectors.begin (), deletedDirectors.end (), celeb));
      if (i != deletedDirectors.end ()) {
	 Check3 (i->isDefined ());
	 movies.scroll_to_row (movies.getModel ()->get_path (movies.append (*i)), 0.8);
	 deletedDirectors.erase (i);
	 msg.replace (msg.find ("%1"), 2, _("director"));
      }
      else {
	 std::map<HInterpret, HInterpret>::iterator i (changedInterprets.find (celeb));
	 if (i != changedInterprets.end ()) {
	    TRACE1 ("CDManager::undo () - Undo to interpret " << i->second->getId () << '/' << i->second->getName ());
	    Gtk::TreeRow row (*movies.getObject (obj));
	    *(i->first) = *(i->second);
	    records.update (row);
	    records.scroll_to_row (records.getModel ()->get_path (row), 0.8);

	    changedInterprets.erase (i);
	    msg.replace (msg.find ("%1"), 2, _("interpret"));
	 }
	 else {
	    std::vector<HInterpret>::iterator i (std::find (deletedInterprets.begin (), deletedInterprets.end (), celeb));
	    Check3 (i != deletedInterprets.end ());
	    Check3 (i->isDefined ());
	    records.scroll_to_row (records.getModel ()->get_path (records.append (*i)), 0.8);
	    deletedInterprets.erase (i);
	    msg.replace (msg.find ("%1"), 2, _("interpret"));
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
   if (undoEntities.empty ()) {
      apMenus[UNDO]->set_sensitive (false);
      apMenus[SAVE]->set_sensitive (false);
   }

   TRACE1 ("CDManager::undo () - Type: " << typeid (*obj).name ());
   status.pop ();
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
