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

#include "smtk/bridge/mesh/Exports.h"

#include "smtk/mesh/Collection.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

#include <vector>

namespace smtk
{
namespace bridge
{
namespace mesh
{

struct SMTKMESHSESSION_EXPORT Topology
{
  Topology(smtk::mesh::CollectionPtr collection);

  struct Element
  {
    Element(int dimension = -1)
      : m_dimension(dimension)
    {
    }

    int m_dimension;
    smtk::common::UUIDArray m_children;
  };

  smtk::mesh::CollectionPtr m_collection;
  std::map<smtk::common::UUID, Element> m_elements;
};
}
}
}

#endif
