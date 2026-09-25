#ifndef FILMDATA_H
#define FILMDATA_H

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

#include <gdkmm/pixbuf.h>

#include <XGP/XDialog.h>

namespace Gtk {
class Label;
class Image;
class TextView;
} // namespace Gtk

/**Dialog allowing to edit icon and description of a film.
 */
class FilmDataEditor : public XGP::XDialog {
  public:
    FilmDataEditor();
    virtual ~FilmDataEditor();

    void setIcon(const std::string& bufImage);
    const std::string getIcon() const;

    void setSummary(const Glib::ustring& summary);
    const Glib::ustring getSummary() const;

    /// Creates the dialog
    /// \remarks Cares also about freeing the dialog
    static FilmDataEditor* create() {
        FilmDataEditor* dlg(new FilmDataEditor);
        dlg->signal_response().connect(sigc::mem_fun(*dlg, &FilmDataEditor::free));
        return dlg;
    }

    // Prohibited manager functions
    FilmDataEditor(const FilmDataEditor&) = delete;
    const FilmDataEditor& operator=(const FilmDataEditor&) = delete;

  protected:
    Gtk::TextView* txtSummary; ///< Field displaying the summary of the plot
    Gtk::Image* image;         ///< Image showing the poster of the film

    void loadIcon();
    void addIcon(const std::string& file);

  private:
    void showPoster(const Glib::RefPtr<Gdk::Pixbuf>& pic);

    Glib::RefPtr<Gdk::Pixbuf> poster; ///< Poster of the film (scaled to the displayed size)
};

#endif
