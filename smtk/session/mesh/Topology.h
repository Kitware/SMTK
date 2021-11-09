//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_mesh_Topology_h
#define smtk_session_mesh_Topology_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

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

   When mapping a mesh resource of mesh sets to a model, it is necessary to
   construct hierarchical relationships between mesh sets. This struct provides
   the description of these relationships, as well as a means of automatically
   constructing the hierarchy by extracting mesh shells.
  */
struct SMTKMESHSESSION_EXPORT Topology
{
  Topology(
    const smtk::common::UUID& modelId,
    const smtk::mesh::MeshSet& meshset,
    bool constructHierarchy = true);

  struct Element
  {
    Element(const smtk::mesh::MeshSet& mesh, const smtk::common::UUID& id, int dimension = -1)
      : m_mesh(mesh)
      , m_dimension(dimension)
      , m_id(id)
    {
    }

    smtk::mesh::MeshSet m_mesh;
    int m_dimension;
    smtk::common::UUID m_id;
    std::set<smtk::common::UUID> m_parents;
    std::set<smtk::common::UUID> m_children;
  };

  smtk::mesh::ResourcePtr m_resource;
  smtk::common::UUID m_modelId;
  std::map<smtk::common::UUID, Element> m_elements;
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif
