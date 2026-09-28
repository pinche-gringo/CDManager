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

#include <stdexcept>
#include <vector>

#include <YGP/Relation.h>

#include "ActorList.h"
#include "Director.h"
#include "Film.h"
#include "RelateFilm.h"

#include "NBPage.h"

// Forward declarations
class Genres;
class PFilms;

/**Class handling the actor notebook-page
 */
class PActors : public NBPage {
  public:
    PActors(Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave, const Genres& genres, PFilms& films);
    ~PActors() override;

    void loadData() override;
    void saveData() override;
    void getFocus() override;
    void addMenu(Glib::RefPtr<Gio::Menu> menuEdit, Glib::RefPtr<Gio::Menu> menuOther,
                 Glib::RefPtr<Gio::SimpleActionGroup> grpAction, Glib::RefPtr<Gtk::ShortcutController> shortcuts) override;
    void deleteSelection() override;
    void undo() override;
    void clear() override;

    [[nodiscard]] HFilm findFilm(unsigned int id) const;

    PActors() = delete;
    PActors(const PActors& other) = delete;
    const PActors& operator=(const PActors& other) = delete;

  private:
    void actorSelected();
    void actorChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue);
    bool onQueryTooltip(int x, int y, bool keyboard, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

    void newActor();
    void undoActor(const Undo& last);

    void actorPlaysInFilm();
    void relateFilms(const HActor& actor, const std::vector<HFilm>& films) pre(actor);
    void showFilms(const HActor& actor, const std::vector<HFilm>& newFilms);

    void changeView(const Glib::ustring& view);
    void viewByActor();
    void viewByFilm();

    void changeAllEntries(const HEntity& entry, Gtk::TreeModel::iterator begin, Gtk::TreeModel::iterator end) pre(entry);
    void saveRelatedFilms(const HActor& actor);

    ActorList actors; // GUI-element holding actors

    // Model
    enum { ACTOR, FILMS };

    std::vector<HActor> aActors;
    YGP::RelationN_M<HActor, HFilm> relActors;

    // Reference to film-page
    PFilms& films;

    // Menu (radio-action) for switching view
    unsigned int actView{0};
    Glib::RefPtr<Gio::SimpleAction> menuView;

    // Info for undoing relating actors and films
    class RelUndo : public YGP::Entity {
      public:
        RelUndo() = default;
        RelUndo(const std::vector<HFilm>& aFilms) : films(aFilms) {}
        ~RelUndo() override = default;

        RelUndo(const RelUndo&) = delete;
        RelUndo& operator=(const RelUndo&) = delete;

        void setRelatedFilms(const std::vector<HFilm>& aFilms) { films = aFilms; }
        [[nodiscard]] const std::vector<HFilm>& getRelatedFilms() const { return films; }

      private:
        std::vector<HFilm> films;
    };
};

#endif
