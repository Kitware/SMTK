//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================


#ifndef __smtk_mesh_moab_Types_h
#define __smtk_mesh_moab_Types_h

//these require us to install moab headers, so lets fix that
#include "moab/EntityHandle.hpp"
#include "moab/Range.hpp"

namespace smtk {
namespace mesh {
namespace moab {

typedef ::moab::EntityHandle  Handle;
typedef ::moab::Range         HandleRange;

}
}
}

#endif