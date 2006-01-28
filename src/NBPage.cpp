//$Id: NBPage.cpp,v 1.3 2006/01/28 08:33:47 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : NBPage
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.3 $
//AUTHOR      : Markus Schwab
//CREATED     : 20.01.2006
//COPYRIGHT   : Copyright (C) 2006

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <gtkmm/statusbar.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "NBPage.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
NBPage::~NBPage () {
}


//-----------------------------------------------------------------------------
/// Enables or disables the edit-menus entries according to the selection
/// \param enable: Flag, if menus should be enabled
//-----------------------------------------------------------------------------
void NBPage::enableEdit (SELECTED selected) {
   TRACE9 ("NBPage::enableEdit (SELECTED) - " << selected);
   Check2 (apMenus[NEW1]); Check2 (apMenus[NEW2]);

   apMenus[DELETE]->set_sensitive (selected != NONE_SELECTED);
   apMenus[NEW1]->set_sensitive (true);
   apMenus[NEW2]->set_sensitive (selected > NONE_SELECTED);
   if (apMenus[NEW3])
      apMenus[NEW3]->set_sensitive (selected == OBJECT_SELECTED);
}

//-----------------------------------------------------------------------------
/// Changes the text of the status-line
/// \param msgStatus: Text to display in the status-line
//-----------------------------------------------------------------------------
void NBPage::showStatus (const Glib::ustring& msgStatus) {
   statusbar.pop ();
   statusbar.push (msgStatus);
}

//-----------------------------------------------------------------------------
/// Removes any created page-related menus
//-----------------------------------------------------------------------------
void NBPage::removeMenu () {
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd: File-descriptor for exporting
//-----------------------------------------------------------------------------
void NBPage::export2HTML (unsigned int fd) {
}
