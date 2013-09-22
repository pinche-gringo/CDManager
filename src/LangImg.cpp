//PROJECT     : CDManager
//SUBSYSTEM   : LanguageImg
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 18.02.2005
//COPYRIGHT   : Copyright (C) 2005, 2010, 2012

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


#include <glibmm/fileutils.h>

#include <gtkmm/misc.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "LangImg.h"


gdouble LanguageImg::saveX (-1);
gdouble LanguageImg::saveY (-1);


//-----------------------------------------------------------------------------
/// Defaultconstructor
/// \param lang: Language whose icon should be displayed; if NULL use
///    an international icon
//-----------------------------------------------------------------------------
LanguageImg::LanguageImg (const char* lang){
   add (img);
   update (lang);
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param file: Name of file; if it is not an absolut path, search in DATADIR
//-----------------------------------------------------------------------------
LanguageImg::LanguageImg (const std::string& file) {
   add (img);
   update (file);
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
      img.set (Gdk::Pixbuf::create_from_file (path.c_str ()));
      img.show ();
   }
   catch (Gdk::PixbufError& e) {
      TRACE1 ("LanguageImg::update (const std::string&): " <<  e.what ());
   }
   catch (Glib::FileError& e) {
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
/// Callback after releasing the button on a LanguageImg
//-----------------------------------------------------------------------------
bool LanguageImg::on_button_release_event (GdkEventButton* ev) {
   Check1 (ev);
   TRACE9 ("LanguageImg::on_button_release_event (GdkEventButton*) - "
           << ev->button << "; X: " << ev->x - 1 << "; Y: " << ev->y - 1
           << "; W: " << get_width () << "; H: " << get_height ());

   // It button 1 is released within the image: Generate a clicked signal
   if ((ev->button == 1)
       && (((ev->x - saveX) < 3) && ((ev->y - saveY) < 3))) {
      clicked_.emit ();
      on_clicked ();
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Callback after pressing a button on a LanguageImg
//-----------------------------------------------------------------------------
bool LanguageImg::on_button_press_event (GdkEventButton* ev) {
   Check1 (ev);
   TRACE9 ("LanguageImg::on_button_press_event (GdkEventButton*) - "
           << ev->button << "; X: " << ev->x - 1 << "; Y: " << ev->y - 1
           << "; W: " << get_width () << "; H: " << get_height ());

   // It button 1 is pressed within the image: store position
   if ((ev->button == 1)
       && ((ev->x - 1) < get_width ()) && ((ev->y - 1) < get_height ())) {
      saveX = ev->x - 1;
      saveY = ev->y - 1;
   }
   return false;
}
