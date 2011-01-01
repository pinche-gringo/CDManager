//PROJECT     : CDManager
//SUBSYSTEM   : Actor
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 2005-10-18
//COPYRIGHT   : Copyright (C) 2005, 2009 - 2011

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


#include <cdmgr-cfg.h>

#include <gtkmm/stock.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "RelateFilm.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose films should be displayed/changed
/// \param films: Films already connected with the actor
/// \param allFilms: All available films
//-----------------------------------------------------------------------------
RelateFilm::RelateFilm (const HActor& actor, const std::vector<HFilm>& films,
			  const Glib::RefPtr<Gtk::TreeStore> allFilms)
   : XGP::XDialog (OKCANCEL),
     mFilms (Gtk::ListStore::create (colFilms)),
     availFilms (allFilms),
     addFilms (*manage (new Gtk::Button)),
     removeFilms (*manage (new Gtk::Button)),
     lstFilms (*manage (new Gtk::TreeView)),
     lstAllFilms (*manage (new Gtk::TreeView)),
     actor (actor) {
   TRACE9 ("RelateFilm::RelateFilm (const HActor&, const std::vector<HFilm>&, const Glib::RefPtr<Gtk::TreeStore>)");
   Check3 (actor);

   for (std::vector<HFilm>::const_iterator i (films.begin ()); i != films.end (); ++i)
      insertFilm (*i);

   init ();
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param actor: Actor whose films should be displayed/changed
/// \param films: Films already connected with the actor
/// \param allFilms: All available films
//-----------------------------------------------------------------------------
RelateFilm::RelateFilm (const HActor& actor, const Glib::RefPtr<Gtk::TreeStore> allFilms)
   : XGP::XDialog (OKCANCEL),
     mFilms (Gtk::ListStore::create (colFilms)),
     availFilms (allFilms),
     addFilms (*manage (new Gtk::Button)),
     removeFilms (*manage (new Gtk::Button)),
     lstFilms (*manage (new Gtk::TreeView)),
     lstAllFilms (*manage (new Gtk::TreeView)),
     actor (actor) {
   TRACE9 ("RelateFilm::RelateFilm (const HActor&, const Glib::RefPtr<Gtk::TreeStore>)");
   init ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RelateFilm::~RelateFilm () {
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void RelateFilm::okEvent () {
   Check3 (actor);

   std::vector<HFilm> films;
   for (Gtk::TreeModel::const_iterator i (mFilms->children ().begin ());
	i != mFilms->children ().end (); ++i)
      films.push_back ((*i)[colFilms.hFilm]);
   signalRelateFilms.emit (actor, films);
}

//-----------------------------------------------------------------------------
/// Adds a film to the ones starring the actor
/// \param path: Path to film to add
//-----------------------------------------------------------------------------
void RelateFilm::addFilm (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
   TRACE7 ("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");
   Check3 (lstAllFilms.get_model ());

   Gtk::TreeIter sel (availFilms->get_iter (path)); Check3 (sel);
   if (sel->parent ()) {
      HEntity entry ((*sel)[colAllFilms.entry]); Check3 (entry);
      HFilm film (boost::dynamic_pointer_cast<Film> (entry)); Check3 (film);
      TRACE9 ("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << film->getName ());
      insertFilm (film);
   }
   else {
      for (Gtk::TreeIter i (sel->children ().begin ()); i != sel->children ().end (); ++i) {
	 HEntity entry ((*i)[colAllFilms.entry]); Check3 (entry);
	 HFilm film (boost::dynamic_pointer_cast<Film> (entry)); Check3 (film);
	 TRACE9 ("RelateFilm::addFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) - " << film->getName ())
	 insertFilm (film);
      }
   }
}

//-----------------------------------------------------------------------------
/// Adds a film to the ones starring the actor
/// \param path: Path to film to add
//-----------------------------------------------------------------------------
void RelateFilm::removeFilm (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
   TRACE9 ("RelateFilm::removeFilm (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)");

   mFilms->erase (mFilms->get_iter (path));
}

//-----------------------------------------------------------------------------
/// Adds the passed film/director to the film-list for the actor
/// \param film: Film to add
/// \param director: Director of the film
//-----------------------------------------------------------------------------
void RelateFilm::insertFilm (const HFilm& film) {
   TRACE9 ("RelateFilm::insertFilm (const HFilm&) - " << (film ? film->getName ().c_str () : ""));
   Check1 (film);

   // Check that film does not exist
   for (Gtk::TreeModel::const_iterator i (mFilms->children ().begin ());
	i != mFilms->children ().end (); ++i)
      if (film == (HFilm)(*i)[colFilms.hFilm])
	 return;

   Gtk::TreeModel::Row newFilm (*mFilms->append ());
   newFilm[colFilms.hFilm] = film;
   newFilm[colFilms.film] = film->getName ();
}

//-----------------------------------------------------------------------------
/// Adds all selected films
//-----------------------------------------------------------------------------
void RelateFilm::addSelected () {
   TRACE9 ("RelateFilm::addSelected ()");

   Glib::RefPtr<Gtk::TreeSelection> filmSel (lstAllFilms.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (filmSel->get_selected_rows ());
   for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
	i != list.end (); ++i)
      addFilm (*i, NULL);
}

//-----------------------------------------------------------------------------
/// Remove all selected films
//-----------------------------------------------------------------------------
void RelateFilm::removeSelected () {
   TRACE9 ("RelateFilm::removeSelected ()");

   Glib::RefPtr<Gtk::TreeSelection> filmSel (lstFilms.get_selection ());
   Gtk::TreeSelection::ListHandle_Path list (filmSel->get_selected_rows ());
   for (Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());
	i != list.end (); ++i)
      removeFilm (*i, NULL);
}

//-----------------------------------------------------------------------------
/// Callback after changing the films-selection. Used to enable/disable the
/// remove-button
//-----------------------------------------------------------------------------
void RelateFilm::filmsSelected () {
   removeFilms.set_sensitive (lstFilms.get_selection ()->get_selected_rows ().size ());
}

//-----------------------------------------------------------------------------
/// Callback after changing the allFilms-selection. Used to enable/disable the
/// add-button
//-----------------------------------------------------------------------------
void RelateFilm::allFilmsSelected () {
   addFilms.set_sensitive (lstAllFilms.get_selection ()->get_selected_rows ().size ());
}

//-----------------------------------------------------------------------------
/// Inititalizes the class
//-----------------------------------------------------------------------------
void RelateFilm::init () {
   Check2 (actor);
   Glib::ustring title (_("Films starring %1"));
   title.replace (title.find ("%1"), 2, actor->getName ());
   set_title (title);
   set_default_size (580, 450);

   Check1 (mFilms);
   lstFilms.set_model (mFilms);
   lstAllFilms.set_model (availFilms);

   Gtk::ScrolledWindow& scrlFilms (*manage (new Gtk::ScrolledWindow));
   Gtk::ScrolledWindow& scrlAllFilms (*manage (new Gtk::ScrolledWindow));
   scrlFilms.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlAllFilms.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlFilms.add (lstFilms);
   scrlAllFilms.add (lstAllFilms);
   scrlFilms.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlAllFilms.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   lstFilms.append_column (_("Film"), colFilms.film);
   lstAllFilms.append_column (_("Available directors/films"), colAllFilms.name);

   Glib::RefPtr<Gtk::TreeSelection> sel (lstFilms.get_selection ());
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &RelateFilm::filmsSelected));
   filmsSelected ();

   sel = lstAllFilms.get_selection ();
   sel->set_mode (Gtk::SELECTION_EXTENDED);
   sel->signal_changed ().connect (mem_fun (*this, &RelateFilm::allFilmsSelected));
   allFilmsSelected ();

   lstFilms.signal_row_activated ().connect (mem_fun (*this, &RelateFilm::removeFilm));
   lstAllFilms.signal_row_activated ().connect (mem_fun (*this, &RelateFilm::addFilm));
   lstAllFilms.expand_all ();

   Gtk::Box& bbox (*manage (new Gtk::VBox));
   addFilms.add (*manage (new Gtk::Image (Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_BUTTON)));
   removeFilms.add (*manage (new Gtk::Image (Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON)));

   bbox.pack_start (addFilms, Gtk::PACK_SHRINK, 5);
   bbox.pack_start (removeFilms, Gtk::PACK_SHRINK, 5);
   addFilms.signal_clicked ().connect (mem_fun (*this, &RelateFilm::addSelected));
   removeFilms.signal_clicked ().connect (mem_fun (*this, &RelateFilm::removeSelected));

   Gtk::HBox& box (*manage (new Gtk::HBox));
   box.pack_start (scrlFilms, Gtk::PACK_EXPAND_WIDGET, 5);
   box.pack_start (bbox, Gtk::PACK_SHRINK, 5);
   box.pack_start (scrlAllFilms, Gtk::PACK_EXPAND_WIDGET, 5);
   get_vbox ()->pack_start (box, Gtk::PACK_EXPAND_WIDGET);

   show_all_children ();
   show ();
}
