//PROJECT     : CDManager
//SUBSYSTEM   : LanguageImg
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 18.02.2005
//COPYRIGHT   : Copyright (C) 2005, 2010, 2012, 2026

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


#include <glibmm/error.h>

#include <gdkmm/texture.h>

#include <gtkmm/gestureclick.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "LangImg.h"


//-----------------------------------------------------------------------------
/// Defaultconstructor
/// \param lang: Language whose icon should be displayed; if NULL use
///    an international icon
//-----------------------------------------------------------------------------
LanguageImg::LanguageImg (const char* lang){
   init ();
   update (lang);
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param file: Name of file; if it is not an absolut path, search in DATADIR
//-----------------------------------------------------------------------------
LanguageImg::LanguageImg (const std::string& file) {
   init ();
   update (file);
}

//-----------------------------------------------------------------------------
/// Initializes the object: Adds the image and the handling of clicks
//-----------------------------------------------------------------------------
void LanguageImg::init () {
   img.set_can_shrink (false);             // Display the flag in its real size
   append (img);

   Glib::RefPtr<Gtk::GestureClick> click (Gtk::GestureClick::create ());
   click->set_button (GDK_BUTTON_PRIMARY);
   click->signal_released ().connect (sigc::mem_fun (*this, &LanguageImg::onReleased));
   add_controller (click);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
LanguageImg::~LanguageImg () {
   TRACE9 ("LanguageImg::~LanguageImg ()");
}


//-----------------------------------------------------------------------------
/// Loads the image from the passed file
/// \param file: Name of file; if it is not an absolut path, search in DATADIR
//-----------------------------------------------------------------------------
void LanguageImg::update (const std::string& file) {
   TRACE2 ("LanguageImg::update (const std::string&) - " << file);
   Check1 (file.size ());

   std::string path;
   if (file[0] != YGP::File::DIRSEPARATOR) {
      path = DATADIR; Check3 (path.size ());
      if (path[path.size () - 1] != YGP::File::DIRSEPARATOR)
	 path += YGP::File::DIRSEPARATOR;
   }
   path += file;

   try {
      img.hide ();
      TRACE2 ("LanguageImg::update (const std::string&) - Loading: " << path);
      img.set_paintable (Gdk::Texture::create_from_filename (path));
      img.show ();
   }
   catch (Glib::Error& e) {
      TRACE1 ("LanguageImg::update (const std::string&): " <<  e.what ());
   }
   catch (...) {
      TRACE1 ("LanguageImg::update (const std::string&): Unknown error");
   }
}

//-----------------------------------------------------------------------------
/// Loads the image from the passed language
/// \param lang: Language whose icon should be displayed; if NULL use
///    an international icon
//-----------------------------------------------------------------------------
void LanguageImg::update (const char* lang) {
   TRACE1 ("LanguageImg::update (const char*) - " << lang);

   std::string file ((lang && *lang) ? lang :  "in");
   file += ".png";
   update (file);
}

//-----------------------------------------------------------------------------
/// Callback after clicking a LanguageImg
//-----------------------------------------------------------------------------
void LanguageImg::on_clicked () {
   TRACE9 ("LanguageImg::on_clicked ()");
}

//-----------------------------------------------------------------------------
/// Callback after releasing the (first) mouse-button on a LanguageImg; if it
/// is released within the image a clicked signal is generated. (The gesture
/// itself is cancelled, if the pointer was moved too far while pressed.)
/// \param x: X-position of the pointer
/// \param y: Y-position of the pointer
//-----------------------------------------------------------------------------
void LanguageImg::onReleased (int, double x, double y) {
   TRACE9 ("LanguageImg::onReleased (int, 2x double) - X: " << x << "; Y: " << y
           << "; W: " << get_width () << "; H: " << get_height ());

   if ((x >= 0) && (y >= 0) && (x < get_width ()) && (y < get_height ())) {
      clicked_.emit ();
      on_clicked ();
   }
}
