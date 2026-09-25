#ifndef SETTINGS_H
#define SETTINGS_H

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

#include <gtkmm/entry.h>

#include <XGP/XAttrEntry.h>

#include <XGP/XDialog.h>

// Forward declarations
class Options;

class Settings : public XGP::XDialog {
  public:
    virtual ~Settings();

    static Settings* create(Gtk::Window& parent, Options& options);

    // Prohibited manager functions
    Settings(const Settings& other) = delete;
    const Settings& operator=(const Settings& other) = delete;

  protected:
    Settings(Options& options);

  private:
    void okEvent() override;

    XGP::XAttributeEntry<std::string> txtOutput;
    XGP::XAttributeEntry<std::string> hdrFilm;
    XGP::XAttributeEntry<std::string> ftrFilm;
    XGP::XAttributeEntry<std::string> hdrRecord;
    XGP::XAttributeEntry<std::string> ftrRecord;

    Gtk::Widget* wordDialog;

    static XGP::XAttributeEntry<std::string> Settings::* fields[];
    static Settings* instance;
};

#endif
