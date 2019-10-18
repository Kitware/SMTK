//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/moab/ModelEntityPointLocator.h"

#include "smtk/AutoInit.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/core/TypeSet.h"

#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/model/EntityRef.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{
ModelEntityPointLocator::ModelEntityPointLocator()
{
}

ModelEntityPointLocator::~ModelEntityPointLocator()
{
}

bool ModelEntityPointLocator::closestPointOn(const smtk::model::EntityRef& entity,
  std::vector<double>& closestPoints, const std::vector<double>& sourcePoints)
{
  // Attempt to access the entity's mesh tessellation
  smtk::mesh::MeshSet meshTessellation = entity.meshTessellation();

  // If the entity has a mesh tessellation, and the mesh backend is moab, and
  // the tessellation has triangles...
  if (meshTessellation.isValid() && meshTessellation.resource()->interfaceName() == "moab" &&
    meshTessellation.types().hasCell(smtk::mesh::Triangle))
  {
    //...then we can use Moab's AdaptiveKDTree to find closest points.
    const smtk::mesh::moab::InterfacePtr& interface =
      std::static_pointer_cast<smtk::mesh::moab::Interface>(
        meshTessellation.resource()->interface());

    // This option restricts the KD tree from subdividing too much
    ::moab::FileOptions treeOptions("MAX_DEPTH=13");

    // Construct an AdaptiveKDTree
    ::moab::EntityHandle treeRootSet;
    ::moab::AdaptiveKDTree tree(interface->moabInterface(),
      smtkToMOABRange(meshTessellation.cells().range()), &treeRootSet, &treeOptions);

    // Prepare the output for its points
    closestPoints.resize(sourcePoints.size());

    // For each point, query the tree for the nearest point.
    ::moab::EntityHandle triangleOut;
    for (std::size_t i = 0; i < sourcePoints.size(); i += 3)
    {
      tree.closest_triangle(treeRootSet, &sourcePoints[i], &closestPoints[i], triangleOut);
    }
    return true;
  }
  return false;
}
}
}
}

smtkDeclareExtension(
  SMTKCORE_EXPORT, model_entity_point_locator, smtk::mesh::moab::ModelEntityPointLocator);

smtkComponentInitMacro(smtk_model_entity_point_locator_extension);
