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

#ifndef smtk_mesh_moab_HandleRangeToRange_h
#define smtk_mesh_moab_HandleRangeToRange_h

#include "smtk/CoreExports.h"

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

SMTKCORE_EXPORT
smtk::mesh::HandleRange moabToSMTKRange(const ::moab::Range&);

SMTKCORE_EXPORT
::moab::Range smtkToMOABRange(const smtk::mesh::HandleRange&);
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
