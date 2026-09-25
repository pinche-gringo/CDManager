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

#include <gtkmm/box.h>
#include <gtkmm/picture.h>


/**Class to display an language-image in the statusbar

  This is actually a box with a click-gesture and not a button, to avoid
  side-effects caused by the theme.
 */
class LanguageImg : public Gtk::Box {
 public:
   LanguageImg (const std::string& file);
   LanguageImg (const char* lang = nullptr);
   ~LanguageImg ();

   void update (const std::string& file);
   void update (const char* lang = nullptr);

   sigc::signal<void ()> signal_clicked () { return clicked_; }

 protected:
  virtual void on_clicked ();

 private:
   void init ();
   void onReleased (int nPress, double x, double y);

   sigc::signal<void ()> clicked_;
   Gtk::Picture img;
};

#endif
