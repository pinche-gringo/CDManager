#ifndef PACTORS_H
#define PACTORS_H

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.


#include <vector>
#include <stdexcept>

#include <gtkmm/radioaction.h>

#include <YGP/Relation.h>

#include "Film.h"
#include "Director.h"
#include "ActorList.h"
#include "RelateFilm.h"

#include "NBPage.h"


// Forward declarations
class Genres;
class PFilms;


/**Class handling the actor notebook-page
 */
class PActors : public NBPage {
 public:
   PActors (Gtk::Statusbar& status, Glib::RefPtr<Gtk::Action> menuSave,
	    const Genres& genres, PFilms& films);
   virtual ~PActors ();

   virtual void loadData ();
   virtual void saveData () throw (std::exception);
   virtual void getFocus ();
   virtual void addMenu (Glib::ustring& ui, Glib::RefPtr<Gtk::ActionGroup> grpAction);
   virtual void deleteSelection ();
   virtual void undo ();
   virtual void clear ();

   HFilm findFilm (unsigned int id) const;

 private:
   PActors ();

   PActors (const PActors& other);
   const PActors& operator= (const PActors& other);

   void actorSelected ();
   void actorChanged (const Gtk::TreeIter& row, unsigned int column, Glib::ustring& oldValue);

   void newActor ();
   void undoActor (const Undo& last);

   void actorPlaysInFilm ();
   void relateFilms (const HActor& actor, const std::vector<HFilm>& films);
   void showFilms (const HActor& actor, const std::vector<HFilm>& newFilms);

   void viewByActor ();
   void viewByFilm ();

   void changeAllEntries (const HEntity& entry, Gtk::TreeIter begin, Gtk::TreeIter end);
   void saveRelatedFilms (const HActor& actor) throw (std::exception);

   ActorList actors;                              // GUI-element holding actors

   // Model
   enum { ACTOR, FILMS };

   std::vector<HActor> aActors;
   YGP::RelationN_M<HActor, HFilm> relActors;

   // Reference to film-page
   PFilms& films;

   // Menus for switching view
   unsigned int actView;
   Glib::RefPtr<Gtk::RadioAction> menuView[2];

   // Info for undoing relating actors and films
   class RelUndo : public YGP::Entity {
    public:
      RelUndo () { }
      RelUndo (const std::vector<HFilm>& aFilms) : films (aFilms) { }
      ~RelUndo () { }

      void setRelatedFilms (const std::vector<HFilm>& aFilms) { films = aFilms; }
      const std::vector<HFilm>& getRelatedFilms () const { return films; }

    private:
      RelUndo (const RelUndo&);
      RelUndo& operator= (const RelUndo&);

      std::vector<HFilm> films;
   };
};

#endif
