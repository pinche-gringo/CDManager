#ifndef LANGUAGEIMG_H
#define LANGUAGEIMG_H

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


#include <string>

#include <gdkmm/pixbuf.h>

#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>


/**Class to display an language-image in the statusbar

  This is actually an event-box and not a button, to avoid
  side-effects caused by the theme.
 */
class LanguageImg : public Gtk::EventBox {
 public:
   LanguageImg (const std::string& file);
   LanguageImg (const char* lang = NULL);
   ~LanguageImg ();

   void update (const std::string& file);
   void update (const char* lang = NULL);

   sigc::signal0<void> signal_clicked () { return clicked_; }

 protected:
  virtual void on_clicked ();
  virtual bool on_button_press_event (GdkEventButton* ev);
  virtual bool on_button_release_event (GdkEventButton* ev);

 private:
   sigc::signal0<void> clicked_;
   Gtk::Image img;

   static gdouble saveX, saveY;
};

#endif
