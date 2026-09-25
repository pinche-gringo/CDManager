#ifndef STATISTICS_H
#define STATISTICS_H

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

#include <XGP/XDialog.h>

namespace Gtk {
class Grid;
}

/**Dialog to display statistical information about the CDMedia database
 */
class Statistics : public XGP::XDialog {
  public:
    static Statistics* create(Gtk::Window& parent);

    virtual ~Statistics();

    // Prohibited manager functions
    Statistics(const Statistics& other) = delete;
    const Statistics& operator=(const Statistics& other) = delete;

  protected:
    Statistics();

  private:
    void addLine(unsigned int line, const Glib::ustring& title1, int value1, const Glib::ustring& title2 = Glib::ustring(),
                 int value2 = 0);

    Gtk::Grid* pClient;

    static Statistics* instance;
};

#endif
