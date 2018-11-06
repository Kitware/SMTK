//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_PropertyData_h
#define __smtk_mesh_PropertyData_h

#include "smtk/SystemConfig.h"

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/StringData.h"

namespace smtk
{
namespace mesh
{

typedef std::map<smtk::mesh::MeshSet, smtk::model::FloatData> MeshFloatData;
typedef std::map<smtk::common::UUID, MeshFloatData> ResourceToMeshFloatData;
typedef std::map<smtk::mesh::MeshSet, smtk::model::StringData> MeshStringData;
typedef std::map<smtk::common::UUID, MeshStringData> ResourceToMeshStringData;
typedef std::map<smtk::mesh::MeshSet, smtk::model::IntegerData> MeshIntegerData;
typedef std::map<smtk::common::UUID, MeshIntegerData> ResourceToMeshIntegerData;

} // namespace mesh
} // namespace smtk

#endif // __smtk_mesh_FloatData_h
