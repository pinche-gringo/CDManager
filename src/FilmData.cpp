//PROJECT     : CDManager
//SUBSYSTEM   : Films
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 30.10.2019
//COPYRIGHT   : Copyright (C) 2019

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

#include <glibmm/fileutils.h>

#include <gdkmm/pixbufloader.h>

#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/StatusObj.h>

#include <XGP/XFileDlg.h>
#include <XGP/MessageDlg.h>

#include "FilmData.h"


static const unsigned int WIDTH = 87;
static const unsigned int HEIGHT = 128;


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
FilmDataEditor::FilmDataEditor()
   : XGP::XDialog(XGP::XDialog::OKCANCEL), txtSummary(new Gtk::TextView()),
     image(new Gtk::Image()) {

   txtSummary->set_wrap_mode(Gtk::WRAP_WORD);
   txtSummary->set_size_request(350, 150);

   Gtk::HBox* hbox(new Gtk::HBox());
   Gtk::VBox* vbox(new Gtk::VBox());

   Gtk::Button* img(new Gtk::Button());
   img->set_size_request(WIDTH, HEIGHT);
   image->set_from_icon_name("image-missing", Gtk::IconSize(6));
   image->set_size_request(WIDTH, HEIGHT);
   img->set_image(*image);

   img->signal_clicked().connect(mem_fun(*this, &FilmDataEditor::loadIcon));

   Gtk::Label* lbl(new Gtk::Label(_("Plot summary:")));
   vbox->pack_start(*manage(lbl), false, false);
   vbox->pack_start(*manage(txtSummary), true, true, 5);
   hbox->pack_start(*manage(vbox), true, true, 5);
   hbox->pack_start(*manage(img), false, false, 5);

   get_vbox()->pack_start(*manage(hbox), false, false, 5);

   show_all_children();
   show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
FilmDataEditor::~FilmDataEditor() {
}

//-----------------------------------------------------------------------------
/// Sets the icon of a film
/// \param bufImage Image description
//-----------------------------------------------------------------------------
void FilmDataEditor::setIcon(const std::string& bufImage) {
   TRACE1("FilmDataEditor::setIcon(const std::string&*) - " << bufImage.length());

   Glib::RefPtr<Gdk::PixbufLoader> picLoader(Gdk::PixbufLoader::create());
   try {
      picLoader->write((const guint8*)bufImage.data(), (gsize)bufImage.size());
      picLoader->close();
      TRACE9("Size " << picLoader->get_pixbuf()->get_width() << '/' << picLoader->get_pixbuf()->get_height());
      image->set(picLoader->get_pixbuf()->scale_simple(WIDTH, HEIGHT, Gdk::INTERP_BILINEAR));
      TRACE9("FilmDataEditor::setIcon(const std::string&) - Dimensions: " << image->get_width() << '/' << image->get_height());
      image->show();
   }
   catch(Gdk::PixbufError& e) { }
   catch(Glib::FileError& e) { }
   catch(Glib::Error& e) {
      TRACE1("Error: " << e.what());
   }
}

//-----------------------------------------------------------------------------
/// Returns the icon of the film
/// \returns const std::string& Icon data
//-----------------------------------------------------------------------------
const std::string FilmDataEditor::getIcon() const {
   gchar* buffer(NULL);
   gsize bufSize(0);
   image->get_pixbuf()->save_to_buffer(buffer, bufSize, "jpeg");

   const std::string icon(buffer, bufSize);
   return icon;
}

//-----------------------------------------------------------------------------
/// Sets the summary
/// \param summary Summary of film
//-----------------------------------------------------------------------------
void FilmDataEditor::setSummary(const Glib::ustring& summary) {
   txtSummary->get_buffer()->set_text(summary);
}

//-----------------------------------------------------------------------------
/// Returns the summary of the film
/// \returns const Glib::ustring& Summary of film
//-----------------------------------------------------------------------------
const Glib::ustring FilmDataEditor::getSummary() const {
   return txtSummary->get_buffer()->get_text();
}

//-----------------------------------------------------------------------------
/// Opens the file load dialog to load an icon
//-----------------------------------------------------------------------------
void FilmDataEditor::loadIcon() {
   auto dlg(XGP::FileDialog::create(_("Load icon"), Gtk::FILE_CHOOSER_ACTION_OPEN,
				    XGP::FileDialog::MUST_EXIST));

   dlg->sigSelected.connect(mem_fun(*this, &FilmDataEditor::addIcon));
   dlg->run();
}

//-----------------------------------------------------------------------------
/// Adds the loaded icon
/// \param filename Name of icon file
//-----------------------------------------------------------------------------
void FilmDataEditor::addIcon(const std::string& filename) {
   TRACE9("FilmDataEditor::addIcon(const std::string&) " << filename);
   Glib::RefPtr<Gdk::Pixbuf> img;
   YGP::StatusObject error;
   try {
      img = Gdk::Pixbuf::create_from_file(filename);
   }
   catch(const Glib::FileError& err) { error.setMessage(YGP::StatusObject::ERROR, err.what()); }
   catch(const Gdk::PixbufError& err) { error.setMessage(YGP::StatusObject::ERROR, err.what()); }

   if (img) {
      img = img->scale_simple(WIDTH, HEIGHT, Gdk::INTERP_BILINEAR);
      image->set(img);
   }
   else {
      Glib::ustring msg(_("Error loading icon from file '%1'!"));
      msg.replace (msg.find ("%1"), 2, filename);
      error.generalize(msg);
      XGP::MessageDlg* dlg(XGP::MessageDlg::create(error));
      dlg->set_title(PACKAGE);
      dlg->get_window()->set_transient_for (this->get_window ());
   }
}
