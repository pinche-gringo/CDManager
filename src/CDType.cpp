//$Id: CDType.cpp,v 1.1 2005/01/20 04:54:36 markus Rel $

//PROJECT     : CDManager
//SUBSYSTEM   : Movie
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 29.11.2004
//COPYRIGHT   : Copyright (A) 2004

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


#include "cdmgr-cfg.h"

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
