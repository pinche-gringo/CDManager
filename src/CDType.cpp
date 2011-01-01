//PROJECT     : CDManager
//SUBSYSTEM   : Film
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (C) 2004, 2005, 2010, 2011

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

#include <glibmm/convert.h>

#include "CDType.h"


CDType* CDType::instance (NULL);


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
CDType::CDType () {
   insert (std::make_pair (0, _("Unspecified")));
   insert (std::make_pair (1, _("1 DVD")));
   insert (std::make_pair (2, _("2 DVDs")));
   insert (std::make_pair (3, _("1 CD")));
   insert (std::make_pair (4, _("2 CDs")));
   insert (std::make_pair (5, _("3 CDs")));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDType::~CDType () {
}
