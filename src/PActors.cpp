//$Id: PActors.cpp,v 1.4 2006/01/28 01:17:13 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Actors
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.4 $
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

#if WITH_ACTORS == 1

#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>

#define CHECK 9
#define TRACELEVEL 9
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
		  const Genres& genres, const PMovies& movies)
   : NBPage (status, menuSave), actors (genres), relActors ("actors"),
     relDelActors ("delActors"), movies (movies) {
   TRACE9 ("PActors::PActors (Gtk::Statusbar&, Glib::RefPtr<Gtk::Action>, const Genres&, const PMovies&)");

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
/// \param actor: Handle to changed actor
//-----------------------------------------------------------------------------
void PActors::actorChanged (const HActor& actor) {
   TRACE9 ("PActors::actorChanged (const HActor&)");

   if (changedActors.find (actor) == changedActors.end ()) {
      changedActors[actor] = new Actor (*actor);
      enableSave ();
   }
   else
      undoActors.erase (std::find (undoActors.begin (), undoActors.end (), actor));
   apMenus[UNDO]->set_sensitive ();

   undoActors.push_back (actor);
}

//-----------------------------------------------------------------------------
/// Adds a new actor to the list
//-----------------------------------------------------------------------------
void PActors::newActor () {
   HActor actor;
   actor.define ();
   addActor (actor);
}

//-----------------------------------------------------------------------------
/// Adds an actor to the actor listbox
/// \param actor: Handle to the new actor
/// \returns Gtk::TreeIter: Iterator to new added actor
//-----------------------------------------------------------------------------
Gtk::TreeIter PActors::addActor (const HActor& actor) {
   aActors.push_back (actor);

   Gtk::TreeModel::iterator i (actors.append (actor));
   actors.selectRow (i);
   actorChanged (actor);
   return i;
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
   if (!relActors.isRelated (actor))
      relActors.relate (actor, *movies.begin ());
   relActors.getObjects (actor) = movies;

   Gtk::TreeIter i (actors.getOwner (actor)); Check3 (i);
   while (i->children ().size ())
      actors.getModel ()->erase (i->children ().begin ());

   for (std::vector<HMovie>::iterator m (relActors.getObjects (actor).begin ());
	m != relActors.getObjects (actor).end (); ++m)
      actors.append (*m, *i);

   if (std::find (changedRelMovies.begin (), changedRelMovies.end (), actor)
       == changedRelMovies.end ())
      changedRelMovies.push_back (actor);

   enableSave ();
}

//-----------------------------------------------------------------------------
/// Loads the actors from the database
///
/// According to the available information the page of the notebook
/// is created.
//-----------------------------------------------------------------------------
void PActors::loadData () {
   try {
      YGP::StatusObject stat;

      std::vector<HActor> actrs;
      StorageActor::loadActors (actrs, stat);

      if (actrs.size ()) {
	 std::sort (actrs.begin (), actrs.end (), &Actor::compById);

	 std::map<unsigned int, std::vector<unsigned int> > actorMovies;
	 StorageActor::loadActorsInMovies (actorMovies);

	 // Iterate over all actors
	 std::sort (actrs.begin (), actrs.end (), &Actor::compByName);
	 HMovie movie; movie.define ();
	 for (std::vector<HActor>::const_iterator i (actrs.begin ()); i != actrs.end (); ++i) {
	    Check3 (i->isDefined ());
	    Gtk::TreeModel::Row actor (actors.append (*i));

	    // Get the movies the actor played in
	    std::map<unsigned int, std::vector<unsigned int> >::iterator iActor
	       (actorMovies.find ((*i)->getId ()));
	    if (iActor != actorMovies.end ()) {
	       // Get movies to the movie-IDs
	       std::vector<HMovie> movies (iActor->second.size ());
	       for (std::vector<unsigned int>::iterator m (iActor->second.begin ());
		    m != iActor->second.end (); ++m)
		  movies.push_back (findMovie (*m));
	       std::sort (movies.begin (), movies.end (), Movie::compByName);

	       // Add the movies to the actor
	       for (std::vector<HMovie>::iterator m (movies.begin ()); m != movies.end (); ++m) {
		  actors.append (movie, actor);
		  relActors.relate (*i, movie);
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
   ui += ("<menuitem action='Undo'/>"
	  "<separator/>"
	  "<menuitem action='NActor'/>"
	  "<menuitem action='AddMovie'/>"
	  "<separator/>"
	  "<menuitem action='Delete'/>"
	  "</placeholder></menu>");

   grpAction->add (apMenus[UNDO] = Gtk::Action::create ("Undo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &PActors::undo));
   grpAction->add (apMenus[NEW1] = Gtk::Action::create ("NActor", Gtk::Stock::NEW,
							_("New _actor")),
		   Gtk::AccelKey (_("<ctl>N")),
		   mem_fun (*this, &PActors::newActor));
   grpAction->add (apMenus[NEW2] = Gtk::Action::create ("AddMovie", _("_Plays in movie...")),
		   Gtk::AccelKey (_("<ctl><alt>N")),
		   mem_fun (*this, &PActors::actorPlaysInMovie));
   grpAction->add (apMenus[DELETE] = Gtk::Action::create ("Delete", Gtk::Stock::DELETE, _("_Delete")),
		   Gtk::AccelKey (_("<ctl>Delete")),
		   mem_fun (*this, &PActors::deleteSelection));

   actorSelected ();
}

//-----------------------------------------------------------------------------
/// Removes the selected actor from the listbox. Depending movies are disconnected.
//-----------------------------------------------------------------------------
void PActors::deleteSelection () {
   TRACE9 ("PActors::deleteSelection ()");

   Glib::RefPtr<Gtk::TreeSelection> selection (actors.get_selection ());
   Gtk::TreeIter selRow (selection->get_selected ());
   if (selRow) {
      Check3 (!selRow->parent ());
      HActor actor (actors.getActorAt (selRow)); Check3 (actor.isDefined ());
      TRACE9 ("PActors::deleteSelectedActor () - Deleting " << actor->getName ());

      if (relDelActors.isRelated (actor))
	 relDelActors.unrelateAll (actor);

      if (relActors.isRelated (actor)) {
	 relDelActors.getObjects (actor) = relActors.getObjects (actor);
	 relActors.unrelateAll (actor);

	 while (selRow->children ().size ())
	    actors.getModel ()->erase (selRow->children ().begin ());
      }
      actors.getModel ()->erase (selRow);
      undoActors.push_back (actor);

      // Though actor is already removed from the listbox, it still
      // has to be removed from the database
      if (actor->getId ())
	 deletedActors.push_back (actor);

      std::map<HActor, HActor>::iterator iActor (changedActors.find (actor));
      if (iActor != changedActors.end ())
	 changedActors.erase (iActor);
   }
   apMenus[UNDO]->set_sensitive ();
   enableSave ();
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PActors::undo () {
   TRACE9 ("PActors::undo ()");
   Check3 (undoActors.size ());

   HActor actor (undoActors.back ());
   undoActors.pop_back ();

   /* Text like "Undone actor 'Sean Penn'" */
   Glib::ustring msg (_("Undone %1 '%2'"));
   msg.replace (msg.find ("%2"), 2, actor->getName ());

   std::map<HActor, HActor>::iterator i (changedActors.find (actor));
   if (i != changedActors.end ()) {
      TRACE1 ("PActors::undo () - Undo actor " << i->second->getId () << '/' << i->second->getName ());
      Gtk::TreeRow row (*actors.getObject (HEntity::cast (actor)));
      *(i->first) = *(i->second);
      actors.update (row);
      actors.scroll_to_row (actors.getModel ()->get_path (row), 0.8);

      changedActors.erase (i);
      msg.replace (msg.find ("%1"), 2, _("actor"));
   }
   else {
      std::vector<HActor>::iterator i (std::find (deletedActors.begin (), deletedActors.end (), actor));
      Check3 (i != deletedActors.end ());
      Check3 (i->isDefined ());
      actors.scroll_to_row (actors.getModel ()->get_path (actors.append (*i)), 0.8);
      deletedActors.erase (i);
      msg.replace (msg.find ("%1"), 2, _("actor"));
   }
   showStatus (msg);
}

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PActors::clear () {
   relDelActors.unrelateAll ();
   undoActors.clear ();
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PActors::saveData () throw (Glib::ustring) {
   HActor actor;
   try {
      while (changedActors.size ()) {
	 actor = changedActors.begin ()->first;
	 Check3 (actor.isDefined ());
	 Check3 (changedActors.begin ()->second);

	 StorageActor::saveActor (actor);
	 changedActors.erase (changedActors.begin ());
      } // endwhile actors changed
   }
   catch (std::exception& err) {
      Check3 (actor.isDefined ());
      Glib::ustring msg (_("Can't write actor `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, actor->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (changedRelMovies.size ()) {
	 actor = *changedRelMovies.begin ();
	 Check3 (actor.isDefined ()); Check3 (relActors.isRelated (actor));

	 StorageActor::startTransaction ();
	 StorageActor::deleteActorMovies (actor->getId ());

	 for (std::vector<HMovie>::const_iterator m (relActors.getObjects (actor).begin ());
	      m != relActors.getObjects (actor).end (); ++m)
	    StorageActor::saveActorMovie (actor->getId (), (*m)->getId ());
	 StorageActor::commitTransaction ();

	 changedRelMovies.erase (changedRelMovies.begin ());
      } // end-while
   }
   catch (std::exception& err) {
      StorageActor::abortTransaction ();

      Glib::ustring msg (_("Can't write movies for actor `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, actor->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }

   try {
      while (deletedActors.size ()) {
	 HActor actor (*deletedActors.begin ()); Check3 (actor.isDefined ());
	 if (actor->getId ())
	    StorageActor::deleteActor (actor->getId ());

	 relDelActors.unrelateAll (actor);
	 deletedActors.erase (deletedActors.begin ());
      } // endwhile
   }
   catch (std::exception& err) {
      Glib::ustring msg (_("Can't delete actor `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, actor->getName ());
      msg.replace (msg.find ("%2"), 2, err.what ());
      throw (msg);
   }
}

#endif // WITH_ACTORS
