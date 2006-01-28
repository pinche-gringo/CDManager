#ifndef PACTORS_H
#define PACTORS_H

//$Id: PActors.h,v 1.3 2006/01/28 07:47:21 markus Exp $

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

#include <map>
#include <vector>

#include <YGP/Relation.h>

#include "Movie.h"
#include "Director.h"
#include "ActorList.h"
#include "RelateMovie.h"

#include "NBPage.h"


// Forward declarations
class Genres;
class PMovies;


/**Class handling the actor notebook-page
 */
class PActors : public NBPage {
 public:
   PActors (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave,
	    const Genres& genres, PMovies& movies);
   virtual ~PActors ();

   virtual void loadData ();
   virtual void saveData () throw (Glib::ustring);
   virtual void getFocus ();
   virtual void addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction);
   virtual void deleteSelection ();
   virtual void undo ();
   virtual void clear ();

   HMovie findMovie (unsigned int id) const;

 private:
   PActors ();

   PActors (const PActors& other);
   const PActors& operator= (const PActors& other);

   void actorSelected ();
   void actorChanged (const HActor& actor);

   void newActor ();
   Gtk::TreeIter addActor (const HActor& actor);

   void actorPlaysInMovie ();
   void relateMovies (const HActor& actor, const std::vector<HMovie>& movies);

   ActorList actors;                              // GUI-element holding actors

   // Model
   std::vector<HActor> aActors;
   YGP::RelationN_M<HActor, HMovie> relActors;

   // Information for undo
   std::map<HActor, HActor> changedActors;
   std::vector<HActor>      undoActors;
   std::vector<HActor>      deletedActors;
   std::vector<HActor>      changedRelMovies;

   YGP::RelationN_M<HActor, HMovie> relDelActors;

   // Reference to movie-page
   PMovies& movies;
};


#endif // WITH_ACTORS

#endif
