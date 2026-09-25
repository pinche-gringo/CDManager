#ifndef FILMLIST_H
#define FILMLIST_H

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

#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "Director.h"
#include "Film.h"

#include "OOList.h"

class Genres;

/**Class describing the columns in the CDType-model
 */
class TypeColumns : public Gtk::TreeModel::ColumnRecord {
  public:
    TypeColumns() { add(type); }

    Gtk::TreeModelColumn<Glib::ustring> type;
};

/**Class for the columns in the film list
 */
class FilmColumns : public OwnerObjectColumns {
  public:
    FilmColumns() : OwnerObjectColumns() {
        add(type);
        add(lang1);
        add(lang2);
        add(lang3);
        add(lang4);
        add(lang5);
        add(langs);
        add(sub1);
        add(sub2);
        add(sub3);
        add(sub4);
        add(sub5);
        add(sub6);
        add(sub7);
        add(sub8);
        add(sub9);
        add(sub10);
        add(titles);
    }

    Gtk::TreeModelColumn<Glib::ustring> type;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> lang1;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> lang2;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> lang3;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> lang4;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> lang5;
    Gtk::TreeModelColumn<unsigned int> langs;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub1;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub2;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub3;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub4;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub5;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub6;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub7;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub8;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub9;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> sub10;
    Gtk::TreeModelColumn<unsigned int> titles;
};

/**Class to hold a list of films
 */
class FilmList : public OwnerObjectList {
  public:
    FilmList(const Genres& genres);
    virtual ~FilmList();

    Gtk::TreeModel::Row append(const HDirector& director) { return OwnerObjectList::append(director); }
    Gtk::TreeModel::Row append(HFilm& film, Gtk::TreeModel::Row& director);

    HFilm getFilmAt(const Gtk::TreeModel::ConstRow& row) const;
    HFilm getFilmAt(const Gtk::TreeModel::const_iterator& iterator) const { return getFilmAt(*iterator); }
    HDirector getDirectorAt(const Gtk::TreeModel::const_iterator& iterator) const { return getCelebrityAt(iterator); }

    void update(const std::string& lang);

    void update(Gtk::TreeModel::Row& row) override;

    FilmList(const FilmList& other) = delete;
    const FilmList& operator=(const FilmList& other) = delete;

  protected:
    void setName(HEntity& object, const Glib::ustring& value) override;
    void setYear(HEntity& object, const Glib::ustring& value) override;
    void setGenre(HEntity& object, unsigned int value) override;

    Glib::ustring getColumnName() const override;
    int sortEntity(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const override;

    void valueChanged(const Glib::ustring& path, const Glib::ustring& value, unsigned int column);

    void onButtonPressed(int nPress, double x, double y);
    void editLanguages(const Glib::ustring& path, bool subtitles);

    void setLanguage(Gtk::TreeModel::Row& row, const std::string& languages);
    void setTitles(Gtk::TreeModel::Row& row, const std::string& titles);

  private:
    TypeColumns colTypes;
    FilmColumns colFilms;

    Glib::RefPtr<Gtk::ListStore> mTypes;
};

#endif
