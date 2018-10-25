//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Topology_h
#define __smtk_session_mesh_Topology_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/MeshSet.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

#include <vector>

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief A tree for representing hierarchical relationships between mesh sets.

   When mapping a collection of mesh sets to a model, it is necessary to
   construct hierarchical relationships between mesh sets. This struct provides
   the description of these relationships, as well as a means of automatically
   constructing the hierarchy by extracting mesh shells.
  */
struct SMTKMESHSESSION_EXPORT Topology
{
  Topology(const smtk::common::UUID& modelId, const smtk::mesh::MeshSet& meshset,
    bool constructHierarchy = true);

  struct Element
  {
    Element(const smtk::mesh::MeshSet& mesh, int dimension = -1)
      : m_mesh(mesh)
      , m_dimension(dimension)
    {
    }

    smtk::mesh::MeshSet m_mesh;
    int m_dimension;
    smtk::common::UUIDArray m_children;
  };

  smtk::mesh::CollectionPtr m_collection;
  smtk::common::UUID m_modelId;
  std::map<smtk::common::UUID, Element> m_elements;
};
}
}
}

#endif
