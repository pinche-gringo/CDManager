//$Id: PActors.cpp,v 1.12 2006/02/27 20:45:35 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Actors
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.12 $
//AUTHOR      : Markus Schwab
//CREATED     : 20.01.2006
//COPYRIGHT   : Copyright (C) 2006

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

#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include "Genres.h"
#include "PMovies.h"
#include "MovieList.h"
#include "StorageActor.h"

#include "PActors.h"


//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling actors
/// \param status: Statusbar to display status-messages
/// \param menuSave: Menu-entry to save the database
/// \param genres: Genres to use in actor-list
/// \param movies: Reference to movie-page
//-----------------------------------------------------------------------------
PActors::PActors (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave,
		  const Genres& genres, PMovies& movies)
   : NBPage (status, menuSave), actors (genres), relActors ("actors"),
     movies (movies) {
   TRACE9 ("PActors::PActors (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&, PMovies&)");

   Gtk::ScrolledWindow* scrl (new Gtk::ScrolledWindow);
   scrl->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrl->add (actors);
   scrl->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   Glib::RefPtr<Gtk::TreeSelection> sel (actors.get_selection ());
   sel->signal_changed ().connect (mem_fun (*this, &PActors::actorSelected));
   actors.signalOwnerChanged.connect (mem_fun (*this, &PActors::actorChanged));

   widget = scrl;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PActors::~PActors () {
}


//-----------------------------------------------------------------------------
/// Callback after selecting an actor
/// \param row: Selected row
//-----------------------------------------------------------------------------
void PActors::actorSelected () {
   TRACE9 ("PActords::actorSelected ()");
   Check3 (actors.get_selection ());

   Gtk::TreeIter s (actors.get_selection ()->get_selected ());
   enableEdit (s ? OWNER_SELECTED : NONE_SELECTED);
}

//----------------------------------------------------------------------------
/// Callback when changing an actor
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PActors::actorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PActors::actorChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)");

   Gtk::TreePath path (actors.getModel ()->get_path (row));
   aUndo.push (Undo (Undo::CHANGED, ACTOR, column,
		     YGP::HEntity::cast (actors.getCelebrityAt (row)), path, oldValue));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Adds a new actor to the list
//-----------------------------------------------------------------------------
void PActors::newActor () {
   HActor actor;
   actor.define ();
   Gtk::TreeModel::iterator i (actors.append (actor));
   Gtk::TreePath path (actors.getModel ()->get_path (i));
   actors.selectRow (i);
   actors.set_cursor (path);

   aUndo.push (Undo (Undo::INSERT, ACTOR, 0, YGP::HEntity::cast (actor), path, ""));

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Displays a dialog enabling to connect movies and actors
//-----------------------------------------------------------------------------
void PActors::actorPlaysInMovie () {
   TRACE9 ("PActors::actorPlaysInMovie ()");
   Check3 (actors.get_selection ());
   Gtk::TreeIter p (actors.get_selection ()->get_selected ()); Check3 (p);
   Check3 (!(*p)->parent ());

   HActor actor (actors.getActorAt (p)); Check3 (actor.isDefined ());
   TRACE9 ("void PActors::actorPlaysInMovie () - Founding actor " << actor->getName ());

   RelateMovie* dlg (relActors.isRelated (actor)
		     ? RelateMovie::create (actor, relActors.getObjects (actor),
					    movies.getMovieList ().getModel ())
		     : RelateMovie::create (actor, movies.getMovieList ().getModel ()));
   dlg->get_window ()->set_transient_for (actors.get_window ());
   dlg->signalRelateMovies.connect (mem_fun (*this, &PActors::relateMovies));
}

//-----------------------------------------------------------------------------
/// Relates movies with an actor
/// \param actor: Actor, to which store movies
/// \param movies: Movies, starring this actor
//-----------------------------------------------------------------------------
void PActors::relateMovies (const HActor& actor, const std::vector<HMovie>& movies) {
   TRACE9 ("PActors::relateMovies (const HActor&, const std::vector<HMovie>&)");
   Check2 (actor.isDefined ());

   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());
   Gtk::TreeIter i (actors.getOwner (actor)); Check3 (i);
   Gtk::TreePath path (model->get_path (i));

   YGP::Handle<RelUndo> hOldRel; hOldRel.define ();
   if (relActors.isRelated (actor)) {
      TRACE5 ("PActors::relateMovies (...) - Storing " << relActors.getObjects (actor).size () << " related movies");
      hOldRel->setRelatedMovies (relActors.getObjects (actor));
   }
   else {
      HMovie movie;
      movie.define ();
      relActors.relate (actor, movie);
   }
   aUndo.push (Undo (Undo::CHANGED, MOVIES, actor->getId (), YGP::HEntity::cast (hOldRel), path, ""));

   delRelation[YGP::HEntity::cast (hOldRel)] = YGP::HEntity::cast (actor);
   relActors.getObjects (actor) = movies;

   showMovies (i);
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Shows the movies of a certain actor
/// \param row: Iterator to actor
//-----------------------------------------------------------------------------
void PActors::showMovies (const Gtk::TreeIter& row) {
   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());

   while (row->children ().size ())
      model->erase (row->children ().begin ());

   HActor actor (actors.getActorAt (row)); Check3 (actor.isDefined ());
   for (std::vector<HMovie>::iterator m (relActors.getObjects (actor).begin ());
	m != relActors.getObjects (actor).end (); ++m)
      actors.append (*m, *row);
}

//-----------------------------------------------------------------------------
/// Loads the actors from the database
///
/// According to the available information the page of the notebook
/// is created.
//-----------------------------------------------------------------------------
void PActors::PActors::loadData () {
   TRACE5 ("PActors::loadData ()");
   if (!movies.isLoaded ())
      movies.loadData ();

   try {
      YGP::StatusObject stat;
      std::vector<HActor> actrs;
      StorageActor::loadActors (actrs, stat);
      TRACE9 ("PActors::loadData () - Actors: " << actrs.size ());

      if (actrs.size ()) {
	 std::sort (actrs.begin (), actrs.end (), &Actor::compByName);

	 std::map<unsigned int, std::vector<unsigned int> > actorMovies;
	 StorageActor::loadActorsInMovies (actorMovies);

	 // Iterate over all actors
	 for (std::vector<HActor>::const_iterator i (actrs.begin ()); i != actrs.end (); ++i) {
	    Check3 (i->isDefined ());
	    Gtk::TreeModel::Row actor (actors.append (*i));

	    // Get the movies the actor played in
	    std::map<unsigned int, std::vector<unsigned int> >::iterator iActor
	       (actorMovies.find ((*i)->getId ()));
	    if (iActor != actorMovies.end ()) {
	       // Get movies to the movie-IDs
	       std::vector<HMovie> movies;
	       movies.reserve (iActor->second.size ());
	       for (std::vector<unsigned int>::iterator m (iActor->second.begin ());
		    m != iActor->second.end (); ++m) {
		  HMovie movie (findMovie (*m));
		  if (movie.isDefined ())
		     movies.push_back (movie);
		  else {
		     Glib::ustring err (_("The database contains an invalid reference (%1) to a movie!"));
		     err.replace (err.find ("%1"), 2, YGP::ANumeric::toString (*m));
		     throw std::runtime_error (err);
		  }
	       }

	       // Add the movies to the actor
	       std::sort (movies.begin (), movies.end (), Movie::compByName);
	       for (std::vector<HMovie>::iterator m (movies.begin ()); m != movies.end (); ++m) {
		  Check (m->isDefined ());
		  actors.append (*m, actor);
		  relActors.relate (*i, *m);
	       } // end-for all movies for an actor
	       actorMovies.erase (iActor);
	    } // end-if director has actors for movies
	 } // end-for all actors
	 actors.expand_all ();
      } // endif actors available

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 actor", "Loaded %1 actors", actrs.size ())));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (actrs.size ()));
      showStatus (msg);

      loaded = true;

      if (stat.getType () > YGP::StatusObject::UNDEFINED) {
	 stat.generalize (_("Warnings loading actors!"));
	 XGP::MessageDlg::create (stat);
      }
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't query the actors1!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Sets the focus to the actor-list
//-----------------------------------------------------------------------------
void PActors::getFocus () {
   actors.grab_focus ();
}

//-----------------------------------------------------------------------------
/// Finds the movie with the passed id
/// \param id: Id of movie to find
/// \returns HMovie: Found movie (undefined, if not found)
//-----------------------------------------------------------------------------
HMovie PActors::findMovie (unsigned int id) const {
   return PMovies::findMovie (movies.getDirectors (), movies.getRelMovies (), id);
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param ui: User-interface string holding menus
/// \param grpActions: Added actions
//-----------------------------------------------------------------------------
void PActors::addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction) {
   ui += ("<menuitem action='AUndo'/>"
	  "<separator/>"
	  "<menuitem action='NActor'/>"
	  "<menuitem action='AddMovie'/>"
	  "<separator/>"
	  "<menuitem action='ADelete'/>"
	  "</placeholder></menu>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("AUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &PActors::undo));
   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NActor", Gtk::Stock::NEW,
							_("New _actor")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PActors::newActor));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("AddMovie", _("_Plays in movie...")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PActors::actorPlaysInMovie));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("ADelete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PActors::deleteSelection));

   apMenus[UNDO]->set_sensitive (false);
   actorSelected ();
}

//-----------------------------------------------------------------------------
/// Removes the selected actor from the listbox. Depending movies are disconnected.
//-----------------------------------------------------------------------------
void PActors::deleteSelection () {
   TRACE9 ("PActors::deleteSelection ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (actors.get_selection ());
   Gtk::TreeIter selRow (selection->get_selected ());
   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());
   Gtk::TreePath path (model->get_path (selRow));

   if (selRow) {
      Check3 (!selRow->parent ());
      HActor actor (actors.getActorAt (selRow)); Check3 (actor.isDefined ());
      TRACE9 ("PActors::deleteSelectedActor () - Deleting " << actor->getName ());

      YGP::Handle<RelUndo> hOldRel; hOldRel.define ();
      if (relActors.isRelated (actor)) {
	 hOldRel->setRelatedMovies (relActors.getObjects (actor));

	 relActors.unrelateAll (actor);

	 while (selRow->children ().size ())
	    model->erase (selRow->children ().begin ());
      }

      delRelation[YGP::HEntity::cast (hOldRel)] = YGP::HEntity::cast (actor);
      aUndo.push (Undo (Undo::DELETE, MOVIES, actor->getId (), YGP::HEntity::cast (hOldRel), path, ""));

      Gtk::TreePath path (model->get_path (selRow));
      aUndo.push (Undo (Undo::DELETE, ACTOR, actor->getId (), YGP::HEntity::cast (actor), path, ""));
      model->erase (selRow);
   }
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PActors::undo () {
   TRACE4 ("PActors::undo () - " << aUndo.size ());
   Check3 (aUndo.size ());

   Undo last (aUndo.top ());
   TRACE9 ("PActors::undo () - Undoing " << last.what () << ": " << last.how ());
   switch (last.what ()) {
   case ACTOR:
      undoActor (last);
      break;

   case MOVIES: {
      Check3 ((last.how () == Undo::CHANGED) || (last.how () == Undo::DELETE));
      Check3 (typeid (*last.getEntity ()) == typeid (RelUndo));
      YGP::Handle<RelUndo> relActor (YGP::Handle<RelUndo>::cast (last.getEntity ()));

      std::map<YGP::HEntity, YGP::HEntity>::iterator delRel
	 (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Actor));
      HActor actor (HActor::cast (delRel->second));

      TRACE5 ("PActors::undo () - Relate to " << relActor->getRelatedMovies ().size () << " movies");
      if (!relActors.isRelated (actor))
	 relActors.relate (actor, *relActor->getRelatedMovies ().begin ());
      relActors.getObjects (actor) = relActor->getRelatedMovies ();
      delRelation.erase (delRel);

      showMovies (actors.getModel ()->get_iter (last.getPath ()));
      break; }

   default:
      Check1 (0);
   } // end-switch

   aUndo.pop ();
   if (aUndo.empty ()) {
      enableSave (false);
      apMenus[UNDO]->set_sensitive (false);
   }
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to an actor
/// \param last: Undo-information
//-----------------------------------------------------------------------------
void PActors::undoActor (const Undo& last) {
   TRACE7 ("PActors::undoActor (const Undo&)");

   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (actors.getModel ()->get_iter (path)); Check3 (!iter->parent ());

   Check3 (typeid (*last.getEntity ()) == typeid (Actor));
   HActor actor (HActor::cast (last.getEntity ()));
   TRACE9 ("PActors::undoActor (const Undo&) - " << last.how () << ": " << actor->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      switch (last.column ()) {
      case 0:
	 actor->setName (last.getValue ());
	 break;

      case 1:
	 actor->setLifespan (last.getValue ());
	 break;

      default:
	 Check1 (0);
      } // end-switch
      break;

   case Undo::INSERT:
      Check3 (!relActors.isRelated (actor));
      actors.getModel ()->erase (iter);
      iter = actors.getModel ()->children ().end ();
      break;

   case Undo::DELETE:
      iter = actors.insert (actor, iter);
      path = actors.getModel ()->get_path (iter);
      break;

   default:
      Check1 (0);
   } // end-switch

   if (iter) {
      Gtk::TreeRow row (*iter);
      actors.update (row);
   }
   actors.set_cursor (path);
   actors.scroll_to_row (path, 0.8);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PActors::clear () {
   actors.clear ();
   relActors.unrelateAll ();
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PActors::saveData () throw (Glib::ustring) {
   TRACE9 ("PActors::saveData ()");

   try {
      std::vector<YGP::HEntity> aSaved;
      std::vector<YGP::HEntity>::iterator posSaved (aSaved.end ());

      while (aUndo.size ()) {
	 Undo last (aUndo.top ());

	 posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
	 if ((posSaved == aSaved.end ()) || (*posSaved != last.getEntity ())) {
	    switch (last.what ()) {
	    case MOVIES:
	    case ACTOR: {
	       HActor actor (HActor::cast ((last.what () == ACTOR)
					   ? last.getEntity ()
					   : delRelation[last.getEntity ()]));
	       if (last.how () == Undo::DELETE) {
		  if (actor->getId ()) {
		     Check3 (actor->getId () == last.column ());
		     StorageActor::deleteActor (actor->getId ());
		  }
	       }
	       else {
		  StorageActor::saveActor (actor);

		  // Check if the related movies have been changed
		  YGP::HEntity entityActor (YGP::HEntity::cast (actor));
		  for (std::map<YGP::HEntity, YGP::HEntity>::iterator i (delRelation.begin ());
		       i != delRelation.end (); ++i)
		     if (i->second == entityActor) {
			saveRelatedMovies (actor);
			delRelation.erase (i);
		     }
	       }
	       break; }

	    default:
	       Check1 (0);
	    } // end-switch
	    aSaved.insert (posSaved, last.getEntity ());
	 }
	 aUndo.pop ();
      } // end-while
      Check3 (apMenus[UNDO]);
      apMenus[UNDO]->set_sensitive (false);

      delRelation.clear ();
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Error saving data!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, err.what ());
      throw (msg);
   }
}

//-----------------------------------------------------------------------------
/// Saves the related movies
/// \param actor: Actor whose movies shall be stored
/// \throw std::exception in case of an error
//-----------------------------------------------------------------------------
void PActors::saveRelatedMovies (const HActor& actor) throw (std::exception) {
   TRACE9 ("PActors::saveRelatedMovies (const HActor&)");
   StorageActor::startTransaction ();
   StorageActor::deleteActorMovies (actor->getId ());

   try {
      for (std::vector<HMovie>::const_iterator m (relActors.getObjects (actor).begin ());
	   m != relActors.getObjects (actor).end (); ++m)
	 StorageActor::saveActorMovie (actor->getId (), (*m)->getId ());
      StorageActor::commitTransaction ();
   }
   catch (std::exception& e) {
      StorageActor::abortTransaction ();
      throw e;
   }
}
