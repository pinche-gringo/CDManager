#ifndef PACTORS_H
#define PACTORS_H

//$Id: PActors.h,v 1.9 2006/04/25 03:16:52 markus Rel $

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

#include <gtkmm/radioaction.h>

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
   void actorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);

   void newActor ();
   void undoActor (const Undo& last);

   void actorPlaysInMovie ();
   void relateMovies (const HActor& actor, const std::vector<HMovie>& movies);
   void showMovies (const HActor& actor, const std::vector<HMovie>& newMovies);

   void viewByActor ();
   void viewByMovie ();

   void changeAllEntries (const YGP::HEntity& entry, Gtk::TreeIter begin, Gtk::TreeIter end);
   void saveRelatedMovies (const HActor& actor) throw (std::exception);

   ActorList actors;                              // GUI-element holding actors

   // Model
   enum { ACTOR, MOVIES };

   std::vector<HActor> aActors;
   YGP::RelationN_M<HActor, HMovie> relActors;

   // Reference to movie-page
   PMovies& movies;

   // Menus for switching view
   unsigned int actView;
   Glib::RefPtr<Gtk::RadioAction> menuView[2];

   // Info for undoing relating actors and movies
   class RelUndo : public YGP::Entity {
    public:
      RelUndo () { }
      RelUndo (const std::vector<HMovie>& aMovies) : movies (aMovies) { }
      ~RelUndo () { }

      void setRelatedMovies (const std::vector<HMovie>& aMovies) { movies = aMovies; }
      const std::vector<HMovie>& getRelatedMovies () const { return movies; }

    private:
      RelUndo (const RelUndo&);
      RelUndo& operator= (const RelUndo&);

      std::vector<HMovie> movies;
   };
};

#endif
