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

/*! \file Handle.h */

#include "smtk/CoreExports.h"
#include "smtk/mesh/moab/HandleRange.h"

#include "cJSON.h"

namespace smtk
{
namespace mesh
{

/// A mesh handle is a reference to a single mesh entity (i.e., a meshset, cell, or point).
typedef smtk::mesh::moab::Handle Handle;
/// A mesh handle range refers to a contiguous range of mesh entities.
typedef smtk::mesh::moab::HandleRange HandleRange;

typedef smtk::mesh::moab::HandleRangeInserter HandleRangeInserter;

SMTKCORE_EXPORT cJSON* to_json(const smtk::mesh::HandleRange& range);

SMTKCORE_EXPORT smtk::mesh::HandleRange from_json(cJSON* json);
}
}

#endif
