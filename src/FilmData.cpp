// PROJECT     : CDManager
// SUBSYSTEM   : Films
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 30.10.2019
// COPYRIGHT   : Copyright (C) 2019, 2026

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

#include <glibmm/bytes.h>
#include <glibmm/error.h>

#include <gdkmm/pixbufloader.h>
#include <gdkmm/texture.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>

#include <YGP/Check.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XFileDlg.h>

#include "FilmData.h"

static const unsigned int WIDTH = 87;
static const unsigned int HEIGHT = 128;

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
FilmDataEditor::FilmDataEditor()
    : XGP::XDialog(XGP::XDialog::OKCANCEL), txtSummary(Gtk::make_managed<Gtk::TextView>()),
      image(Gtk::make_managed<Gtk::Image>()) {

    txtSummary->set_wrap_mode(Gtk::WrapMode::WORD);
    txtSummary->set_size_request(350, 150);
    txtSummary->set_expand(true);

    Gtk::Box* hbox(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5));
    Gtk::Box* vbox(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5));

    Gtk::Button* img(Gtk::make_managed<Gtk::Button>());
    img->set_size_request(WIDTH, HEIGHT);
    img->set_valign(Gtk::Align::START);
    image->set_from_icon_name("image-missing");
    image->set_pixel_size(HEIGHT);
    image->set_size_request(WIDTH, HEIGHT);
    img->set_child(*image);

    img->signal_clicked().connect(sigc::mem_fun(*this, &FilmDataEditor::loadIcon));

    Gtk::Label* lbl(Gtk::make_managed<Gtk::Label>(_("Plot summary:")));
    lbl->set_halign(Gtk::Align::START);
    vbox->append(*lbl);
    vbox->append(*txtSummary);
    hbox->append(*vbox);
    hbox->append(*img);
    hbox->set_margin(5);

    get_content_area()->append(*hbox);

    show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
FilmDataEditor::~FilmDataEditor() {}

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
        showPoster(picLoader->get_pixbuf()->scale_simple(WIDTH, HEIGHT, Gdk::InterpType::BILINEAR));
    }
    catch (Glib::Error& e) {
        TRACE1("Error: " << e.what());
    }
}

//-----------------------------------------------------------------------------
/// Stores and displays the (scaled) poster
/// \param pic Poster to display
/// \throw Glib::Error In case of an error
//-----------------------------------------------------------------------------
void FilmDataEditor::showPoster(const Glib::RefPtr<Gdk::Pixbuf>& pic) {
    Check1(pic);

    // Convert the poster to a texture (via PNG, as
    // Gdk::Texture::create_for_pixbuf is deprecated)
    gchar* buffer(nullptr);
    gsize bufSize(0);
    pic->save_to_buffer(buffer, bufSize, "png");
    Glib::RefPtr<Glib::Bytes> bytes(Glib::Bytes::create(buffer, bufSize));
    g_free(buffer);

    image->set(Gdk::Texture::create_from_bytes(bytes));
    poster = pic;
}

//-----------------------------------------------------------------------------
/// Returns the icon of the film
/// \returns const std::string& Icon data (empty, if there is no icon)
//-----------------------------------------------------------------------------
const std::string FilmDataEditor::getIcon() const {
    if (!poster)
        return std::string();

    gchar* buffer(nullptr);
    gsize bufSize(0);
    poster->save_to_buffer(buffer, bufSize, "jpeg");

    const std::string icon(buffer, bufSize);
    g_free(buffer);
    return icon;
}

//-----------------------------------------------------------------------------
/// Sets the summary
/// \param summary Summary of film
//-----------------------------------------------------------------------------
void FilmDataEditor::setSummary(const Glib::ustring& summary) { txtSummary->get_buffer()->set_text(summary); }

//-----------------------------------------------------------------------------
/// Returns the summary of the film
/// \returns const Glib::ustring& Summary of film
//-----------------------------------------------------------------------------
const Glib::ustring FilmDataEditor::getSummary() const { return txtSummary->get_buffer()->get_text(); }

//-----------------------------------------------------------------------------
/// Opens the file load dialog to load an icon
//-----------------------------------------------------------------------------
void FilmDataEditor::loadIcon() {
    auto dlg(XGP::FileDialog::create(_("Load icon"), Gtk::FileChooser::Action::OPEN, XGP::FileDialog::MUST_EXIST));

    dlg->set_transient_for(*this);
    dlg->set_modal(true);
    dlg->sigSelected.connect(sigc::mem_fun(*this, &FilmDataEditor::addIcon));
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
        if (img)
            showPoster(img->scale_simple(WIDTH, HEIGHT, Gdk::InterpType::BILINEAR));
    }
    catch (const Glib::Error& err) {
        error.setMessage(YGP::StatusObject::ERROR, err.what());
        img.reset();
    }

    if (!img) {
        Glib::ustring msg(_("Error loading icon from file '%1'!"));
        msg.replace(msg.find("%1"), 2, filename);
        error.generalize(msg);
        XGP::MessageDlg* dlg(XGP::MessageDlg::create(error));
        dlg->set_title(PACKAGE);
        dlg->set_transient_for(*this);
    }
}
