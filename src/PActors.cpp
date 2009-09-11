//$Id: PActors.cpp,v 1.17 2006/06/06 22:02:03 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Actors
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.17 $
//AUTHOR      : Markus Schwab
//CREATED     : 20.01.2006
//COPYRIGHT   : Copyright (C) 2006, 2009

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

#include <boost/shared_ptr.hpp>

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
#include "SaveCeleb.h"
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
     movies (movies), actView (0) {
   TRACE9 ("PActors::PActors (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&, PMovies&)");

   Gtk::ScrolledWindow* scrl (new Gtk::ScrolledWindow);
   scrl->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrl->add (actors);
   scrl->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   Glib::RefPtr<Gtk::TreeSelection> sel (actors.get_selection ());
   sel->signal_changed ().connect (mem_fun (*this, &PActors::actorSelected));
   actors.signalActorChanged.connect (mem_fun (*this, &PActors::actorChanged));

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
   if (actView) {
      enableEdit ((s && (*s)->parent ()) ? OWNER_SELECTED : NONE_SELECTED);
      apMenus[NEW1]->set_sensitive (false);
   }
   else {
      enableEdit (s ? OWNER_SELECTED : NONE_SELECTED);
      if (s && (*s)->parent ())
	 apMenus[DELETE]->set_sensitive (false);
   }
}

//----------------------------------------------------------------------------
/// Callback when changing an actor
/// \param row: Changed line
/// \param column: Changed column
/// \param oldValue: Old value of the changed entry
//-----------------------------------------------------------------------------
void PActors::actorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue) {
   TRACE9 ("PActors::actorChanged (const Gtk::TreeIter&, unsigned int, Glib::ustring&)");

   Gtk::TreePath path (actors.get_model ()->get_path (row));
   HEntity entity (actors.getEntityAt (row));
   aUndo.push (Undo (Undo::CHANGED, ACTOR, column, entity, path, oldValue));

   if (actView)
      changeAllEntries (entity, actors.getModel ()->children ().begin (), actors.getModel ()->children ().end ());
   std::sort (aActors.begin (), aActors.end (), &Actor::compByName);

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Adds a new actor to the list
//-----------------------------------------------------------------------------
void PActors::newActor () {
   HActor actor (new Actor);
   Gtk::TreeModel::iterator i;
   if (actView) {
      Check3 (actors.get_selection ()->get_selected ());
      i = actors.append (actor, actors.get_selection ()->get_selected ());
   }
   else
      i = actors.append (actor);
   aActors.insert (lower_bound (aActors.begin (), aActors.end (), actor,
				&Actor::compByName), actor);

   Gtk::TreePath path (actors.get_model ()->get_path (i));
   actors.selectRow (i);

   aUndo.push (Undo (Undo::INSERT, ACTOR, 0, actor, path, ""));

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

   HActor actor (boost::dynamic_pointer_cast<Actor> (actors.getEntityAt (p)));
   if (!actor) {
      Check3 ((*p)->parent ());
      p = (*p)->parent ();
      actor = boost::dynamic_pointer_cast<Actor> (actors.getEntityAt (p));
   }
   Check3 (actor);
   TRACE9 ("void PActors::actorPlaysInMovie () - Found actor " << actor->getName ());

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
   Check2 (actor);

   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());
   Gtk::TreeIter i (actors.findEntity (actor)); Check3 (i);
   Gtk::TreePath path (model->get_path (i));

   // Unrelate old movies and relate with newly selected ones
   boost::shared_ptr<RelUndo> hOldRel (new RelUndo);
   if (relActors.isRelated (actor))
      hOldRel->setRelatedMovies (relActors.getObjects (actor));

   aUndo.push (Undo (Undo::CHANGED, MOVIES, actor->getId (), hOldRel, path, ""));
   delRelation[hOldRel] = actor;

   showMovies (actor, movies);

   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Shows the movies of a certain actor
/// \param actor: Actor
/// \param newMovies: Array with new movies
//-----------------------------------------------------------------------------
void PActors::showMovies (const HActor& actor, const std::vector<HMovie>& newMovies) {
   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());
   Gtk::TreeIter i (actors.findEntity (actor)); Check3 (i);

   // Unrelate old movies from actor; in view-by-movie mode this means also
   // remove the actors from the movies
   TRACE5 ("PActors::showMovies (HActor, std::vector<HMovie>) - Relate to " << newMovies.size () << " movies");
   std::vector<Gtk::TreeIter> emptyMovies;
   if (relActors.isRelated (actor)) {
      if (actView) {
	 for (std::vector<HMovie>::const_iterator m (relActors.getObjects (actor).begin ());
	      m != relActors.getObjects (actor).end (); ++m) {
	    Gtk::TreeIter i (actors.findEntity (actor, 2)); Check2 (i);
	    Check3 ((*i)->parent ());
	    Gtk::TreeIter parent ((*i)->parent ()); Check3 (parent);
	    model->erase (i);

	    // Check if parent of deleted actor has now no more childs;
	    // If so, store this movie, to maybe remove it later from the list
	    if (parent->children ().empty ())
	       emptyMovies.push_back (parent);
	 }
      }
      else
	 while ((*i)->children ().size ())
	    model->erase ((*i)->children ().begin ());
      relActors.unrelateAll (actor);
   }

   // Then relate the new (undone) movies with actor; in view-by-movie mode
   // this means also showing the actor for the movies
   for (std::vector<HMovie>::const_iterator m (newMovies.begin ()); m != newMovies.end (); ++m) {
      relActors.relate (actor, *m);

      if (actView) {
	 Gtk::TreeIter iter (*actors.findEntity (*m, 1));
	 if (!iter)
	    iter = actors.append (*m);

	 Gtk::TreeRow row (*iter);
	 actors.append (actor, row);
	 actors.expand_row (model->get_path (row), true);
      }
      else
	 actors.append (*m, *i);
   }

   if (actView) {
      // Remove all movies without actors
      for (std::vector<Gtk::TreeIter>::iterator i (emptyMovies.begin ());
	   i != emptyMovies.end (); ++i)
	 if ((*i)->children ().empty ())
	    model->erase (*i);
   }
   else {
       // In view-by-actor mode the remove/append can be done in one step
      actors.selectRow (i);
      actors.expand_row (model->get_path (i), true);
   }
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
      StorageActor::loadActors (aActors, stat);
      TRACE9 ("PActors::loadData () - Actors: " << aActors.size ());

      if (aActors.size ()) {
	 std::sort (aActors.begin (), aActors.end (), &Actor::compByName);

	 std::map<unsigned int, std::vector<unsigned int> > actorMovies;
	 StorageActor::loadActorsInMovies (actorMovies);

	 // Iterate over all actors
	 for (std::vector<HActor>::const_iterator i (aActors.begin ()); i != aActors.end (); ++i) {
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
		  if (movie)
		     movies.push_back (movie);
		  else {
		     Glib::ustring err (_("The database contains an invalid reference (%1) to a movie!"));
		     err.replace (err.find ("%1"), 2, YGP::ANumeric::toString (*m));
		     throw std::invalid_argument (err);
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

      Glib::ustring msg (Glib::locale_to_utf8 (ngettext ("Loaded %1 actor", "Loaded %1 actors", aActors.size ())));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (aActors.size ()));
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
	  "</placeholder></menu>"
	  "<placeholder name='Other'><menu action='View'>"
	  "<menuitem action='ByActor'/>"
	  "<menuitem action='ByMovie'/>"
	  "</menu></placeholder>");

   // Add edit-menu
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

   // Add view-menu
   grpAction->add (Gtk::Action::create ("View", _("_View")));
   Gtk::RadioButtonGroup grpOrder;
   grpAction->add (menuView[0] = Gtk::RadioAction::create (grpOrder, "ByActor", _("By _actor")),
		   Gtk::AccelKey ("<ctl>1"), mem_fun (*this, &PActors::viewByActor));
   grpAction->add (menuView[1] = Gtk::RadioAction::create (grpOrder, "ByMovie", _("By _movie")),
		   Gtk::AccelKey ("<ctl>2"), mem_fun (*this, &PActors::viewByMovie));

   Check2 (actView < (sizeof (menuView) / sizeof (*menuView)));
   menuView[actView]->set_active ();

   actView ? viewByMovie () : viewByActor ();
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
      HActor actor (boost::dynamic_pointer_cast<Actor> (actors.getEntityAt (selRow))); Check3 (actor);
      TRACE9 ("PActors::deleteSelectedActor () - Deleting " << actor->getName ());

      boost::shared_ptr<RelUndo> hOldRel (new RelUndo);
      if (relActors.isRelated (actor)) {
	 hOldRel->setRelatedMovies (relActors.getObjects (actor));
	 relActors.unrelateAll (actor);

	 while (selRow->children ().size ())
	    model->erase (selRow->children ().begin ());
      }

      delRelation[hOldRel] = actor;
      aUndo.push (Undo (Undo::DELETE, MOVIES, actor->getId (), hOldRel, path, ""));

      Gtk::TreePath path (model->get_path (selRow));
      aUndo.push (Undo (Undo::DELETE, ACTOR, actor->getId (), actor, path, ""));
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
      boost::shared_ptr<RelUndo> relActor (boost::dynamic_pointer_cast<RelUndo> (last.getEntity ()));

      std::map<HEntity, HEntity>::iterator delRel (delRelation.find (last.getEntity ()));
      Check3 (typeid (*delRel->second) == typeid (Actor));
      HActor actor (boost::dynamic_pointer_cast<Actor> (delRel->second));
      Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());

      showMovies (actor, relActor->getRelatedMovies ());
      delRelation.erase (delRel);
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

   Glib::RefPtr<Gtk::TreeStore> model (actors.getModel ());
   Gtk::TreePath path (last.getPath ());
   Gtk::TreeIter iter (model->get_iter (path));

   Check3 (typeid (*last.getEntity ()) == typeid (Actor));
   HActor actor (boost::dynamic_pointer_cast<Actor> (last.getEntity ()));
   TRACE9 ("PActors::undoActor (const Undo&) - " << last.how () << ": " << actor->getName ());

   switch (last.how ()) {
   case Undo::CHANGED:
      switch (last.column ()) {
      case 0:
	 actor->setName (last.getValue ());
	 if (actView)
	    changeAllEntries (actor, model->children ().begin (), model->children ().end ());
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
      model->erase (iter);
      iter = model->children ().end ();
      break;

   case Undo::DELETE:
      iter = actors.insert (actor, iter);
      path = model->get_path (iter);
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
   aActors.clear ();
   actors.clear ();
   actors.getModel ()->clear ();
   relActors.unrelateAll ();
   NBPage::clear ();
   actView = 0;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PActors::saveData () throw (std::exception) {
   TRACE9 ("PActors::saveData ()");

   std::vector<HEntity> aSaved;
   std::vector<HEntity>::iterator posSaved (aSaved.end ());

   while (aUndo.size ()) {
      Undo last (aUndo.top ());

      posSaved = lower_bound (aSaved.begin (), aSaved.end (), last.getEntity ());
      if ((posSaved == aSaved.end ()) || (*posSaved != last.getEntity ())) {
	 switch (last.what ()) {
	 case MOVIES:
	 case ACTOR: {
	    HActor actor (boost::dynamic_pointer_cast<Actor> ((last.what () == ACTOR)
							      ? last.getEntity ()
							      : delRelation[last.getEntity ()]));
	    if (last.how () == Undo::DELETE) {
	       if (actor->getId ()) {
		  Check3 (actor->getId () == last.column ());
		  StorageActor::deleteActor (actor->getId ());
	       }
	    }
	    else {
	       SaveCelebrity::store (actor, "Actors", *getWindow ());

	       // Check if the related movies have been changed
	       HEntity entityActor (actor);
	       for (std::map<HEntity, HEntity>::iterator i (delRelation.begin ());
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

//-----------------------------------------------------------------------------
/// Views the list sorted by actor
//-----------------------------------------------------------------------------
void PActors::viewByActor () {
   if (menuView[0]->get_active ()) {
      TRACE9 ("PActors::viewByActor () - Actors: " << aActors.size ());
      actView = 0;

      Check3 (actors.get_column (0));
      actors.get_column (0)->set_title (_("Actors/Movies"));

      // Fill list sorted by actors with their movies as child
      actors.clear ();
      for (std::vector<HActor>::const_iterator a (aActors.begin ()); a != aActors.end (); ++a) {
	 Gtk::TreeModel::Row actor (actors.append (*a));

	 if (relActors.isRelated (*a)) {
	    const std::vector<HMovie>& am (relActors.getObjects (*a));
	    for (std::vector<HMovie>::const_iterator m (am.begin ()); m != am.end (); ++m)
	       actors.append (*m, actor);
	 }
      }
      actors.expand_all ();
      actorSelected ();
   }
}

//-----------------------------------------------------------------------------
/// Views the list sorted by movie
//-----------------------------------------------------------------------------
void PActors::viewByMovie () {
   if (menuView[1]->get_active ()) {
      actView = 1;

      Check3 (actors.get_column (0));
      actors.get_column (0)->set_title (_("Movies/Actors"));

      std::vector<HMovie> aMovies;
      for (std::vector<HActor>::const_iterator a (aActors.begin ()); a != aActors.end (); ++a)
	 if (relActors.isRelated (*a)) {
	    const std::vector<HMovie>& am (relActors.getObjects (*a));
	    for (std::vector<HMovie>::const_iterator m (am.begin ()); m != am.end (); ++m) {
	       std::vector<HMovie>::iterator pos (lower_bound (aMovies.begin (), aMovies.end (), *m));
	       if ((pos == aMovies.end ()) || (*pos != *m))
		  aMovies.insert (pos, *m);
	    }
	 }
      std::sort (aMovies.begin (), aMovies.end (), &Movie::compByName);

      // Fill list sorted by actors with their movies as child
      actors.clear ();
      for (std::vector<HMovie>::const_iterator m (aMovies.begin ()); m != aMovies.end (); ++m) {
	 Gtk::TreeModel::Row movie (actors.append (*m));

	 if (relActors.isRelated (*m)) {
	    const std::vector<HActor>& aa (relActors.getParents (*m));
	    for (std::vector<HActor>::const_iterator a (aa.begin ()); a != aa.end (); ++a)
	       actors.append (*a, movie);
	 }
      }
      actors.expand_all ();
      actorSelected ();
   }
}

//-----------------------------------------------------------------------------
/// Updates all entries showing the passed entry
/// \param entry: Entry to find
/// \param begin: Start object
/// \param end: End object
//-----------------------------------------------------------------------------
void PActors::changeAllEntries (const HEntity& entry, Gtk::TreeIter begin, Gtk::TreeIter end) {
   Check1 (entry);

   while (begin != end) {
      Gtk::TreeModel::Row row (*begin);
      if (entry == actors.getEntityAt (row))
	 actors.update (row);

      if (begin->children ().size ())
	 changeAllEntries (entry, begin->children ().begin (), begin->children ().end ());
      ++begin;
   } // end-while
}
