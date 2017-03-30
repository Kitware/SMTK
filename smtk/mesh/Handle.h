//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Handle_h
#define __smtk_mesh_Handle_h

#include "smtk/CoreExports.h"
#include "smtk/mesh/moab/HandleRange.h"

#ifndef SHIBOKEN_SKIP
#include "cJSON.h"
#endif // SHIBOKEN_SKIP

namespace smtk
{
namespace mesh
{
typedef smtk::mesh::moab::Handle Handle;
typedef smtk::mesh::moab::HandleRange HandleRange;
typedef smtk::mesh::moab::HandleRangeInserter HandleRangeInserter;

SMTKCORE_EXPORT cJSON* to_json(const smtk::mesh::HandleRange& range);

SMTKCORE_EXPORT smtk::mesh::HandleRange from_json(cJSON* json);
}
}

#endif
