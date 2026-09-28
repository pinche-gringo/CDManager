// PROJECT     : CDManager
// SUBSYSTEM   : Statistics
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 04.04.2010
// COPYRIGHT   : Copyright (C) 2010, 2011, 2026

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

#include <array>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/separator.h>

#include <YGP/ANumeric.h>

#include "Storage.h"

#include "Statistics.h"

//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
Statistics::Statistics() : XGP::XDialog(CANCEL), pClient(Gtk::make_managed<Gtk::Grid>()) {
    set_title(_("Statistical information"));

    pClient->set_column_spacing(10);
    pClient->set_row_spacing(5);
    pClient->set_margin(5);

    Gtk::Label* lbl(Gtk::make_managed<Gtk::Label>(_("The database contains:")));
    lbl->set_margin_bottom(5);
    pClient->attach(*lbl, 0, 0, 4, 1);

    std::array<int, 7> stats{};
    try {
        Storage::getStatistics(stats.data());
    }
    catch (std::exception& err) {
        Gtk::MessageDialog dlg(Glib::ustring::compose(_("Can't query the statistical information!\n\nReason: %1"), err.what()),
                               false, Gtk::MessageType::ERROR);
        XGP::runModal(dlg);
    }

    unsigned int line(1);
    // Add record information
#if WITH_RECORDS
    addLine(line++, _("Interprets:"), stats[2], _("Records:"), stats[3]);
#endif

#if WITH_FILMS
    // Add film information
    addLine(line++, _("Directors:"), stats[4], _("Films:"), stats[5]);
#endif

#if WITH_ACTORS
    // Add film information
    addLine(line++, _("Actors:"), stats[6]);
#endif

    // Add names and articles
#if WITH_RECORDS || WITH_FILMS || WITH_ACTORS
    Gtk::Separator* sep(Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL));
    sep->set_margin_top(5);
    sep->set_margin_bottom(5);
    pClient->attach(*sep, 0, line++, 4, 1);
#endif

    addLine(line, _("First names:"), stats[0], _("Articles:"), stats[1]);

    get_content_area()->append(*pClient);
    show();
}

//-----------------------------------------------------------------------------
/// Adds a line with (one or two) titles and their values
/// \param line Line (in the grid) to add
/// \param title1 First title
/// \param value1 Value to the first title
/// \param title2 Second title; if empty, only the first title is added
/// \param value2 Value to the second title
//-----------------------------------------------------------------------------
void Statistics::addLine(unsigned int line, const Glib::ustring& title1, int value1, const Glib::ustring& title2, int value2) {
    pClient->attach(*Gtk::make_managed<Gtk::Label>(title1), 0, line);
    pClient->attach(*Gtk::make_managed<Gtk::Label>(YGP::ANumeric(value1).toString()), 1, line);
    if (!title2.empty()) {
        pClient->attach(*Gtk::make_managed<Gtk::Label>(title2), 2, line);
        pClient->attach(*Gtk::make_managed<Gtk::Label>(YGP::ANumeric(value2).toString()), 3, line);
    }
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Statistics::~Statistics() { instance = nullptr; }

//-----------------------------------------------------------------------------
/// Creates or selects (if already existing) a dialog to change the
/// preferences.
/// \param parent: Parent window
/// \returns Settings*: Pointer to the created window
//-----------------------------------------------------------------------------
Statistics* Statistics::create(Gtk::Window& parent) {
    if (instance == nullptr) {
        instance = new Statistics();
        instance->set_transient_for(parent);
        instance->signal_response().connect(sigc::mem_fun(*instance, &Statistics::free));
    }
    else
        instance->present();
    return instance;
}
