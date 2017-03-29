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

#include "smtk/mesh/MeshSet.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/StringData.h"

namespace smtk {
  namespace mesh {

#ifdef SMTK_HASH_STORAGE
    typedef google::sparse_hash_map<smtk::mesh::MeshSet,smtk::model::FloatData>   MeshFloatData;
    typedef google::sparse_hash_map<smtk::common::UUID, MeshFloatData>   CollectionToMeshFloatData;
    typedef google::sparse_hash_map<smtk::mesh::MeshSet,smtk::model::StringData>  MeshStringData;
    typedef google::sparse_hash_map<smtk::common::UUID, MeshStringData>  CollectionToMeshStringData;
    typedef google::sparse_hash_map<smtk::mesh::MeshSet,smtk::model::IntegerData> MeshIntegerData;
    typedef google::sparse_hash_map<smtk::common::UUID, MeshIntegerData> CollectionToMeshIntegerData;
#else // SMTK_HASH_STORAGE
    typedef std::map<smtk::mesh::MeshSet,smtk::model::FloatData>   MeshFloatData;
    typedef std::map<smtk::common::UUID, MeshFloatData>   CollectionToMeshFloatData;
    typedef std::map<smtk::mesh::MeshSet,smtk::model::StringData>  MeshStringData;
    typedef std::map<smtk::common::UUID, MeshStringData>  CollectionToMeshStringData;
    typedef std::map<smtk::mesh::MeshSet,smtk::model::IntegerData> MeshIntegerData;
    typedef std::map<smtk::common::UUID, MeshIntegerData> CollectionToMeshIntegerData;
#endif // SMTK_HASH_STORAGE

  } // namespace mesh
} // namespace smtk

#endif // __smtk_mesh_FloatData_h
