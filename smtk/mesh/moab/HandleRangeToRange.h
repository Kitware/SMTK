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

#ifndef __smtk_mesh_moab_HandleRangeToRange_h
#define __smtk_mesh_moab_HandleRangeToRange_h

#include "smtk/mesh/core/Handle.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/EntityHandle.hpp"
#include "moab/Range.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{

//these aren't exported as they are private functions that only
//smtk::mesh should call

smtk::mesh::HandleRange moabToSMTKRange(const ::moab::Range&);

::moab::Range smtkToMOABRange(const smtk::mesh::HandleRange&);
}
}
}

#endif
