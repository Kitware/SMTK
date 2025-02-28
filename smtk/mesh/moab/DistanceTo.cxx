//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/moab/DistanceTo.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/moab/PointLocatorCache.h"

namespace smtk
{
namespace mesh
{
namespace moab
{
std::pair<double, std::array<double, 3>> DistanceTo::operator()(
  const smtk::resource::Component::Ptr& component,
  const std::array<double, 3>& point) const
{
  auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
  if (meshComponent)
  {
    return operator()(meshComponent->mesh(), point);
  }

  auto modelComponent = std::dynamic_pointer_cast<smtk::model::Entity>(component);
  if (modelComponent)
  {
    return operator()(
      modelComponent->referenceAs<smtk::model::EntityRef>().meshTessellation(), point);
  }

  return this->Parent::operator()(component, point);
}

std::pair<double, std::array<double, 3>> DistanceTo::operator()(
  const smtk::mesh::MeshSet& meshset,
  const std::array<double, 3>& point) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  std::pair<double, std::array<double, 3>> returnValue(nan, { { nan, nan, nan } });

  // If the entity has a mesh tessellation, and the mesh backend is moab, and
  // the tessellation has triangles...
  if (
    meshset.isValid() && meshset.resource()->interfaceName() == "moab" &&
    meshset.types().hasCell(smtk::mesh::Triangle))
  {
    PointLocatorCache& pointLocatorCache = meshset.resource()->queries().cache<PointLocatorCache>();

    //...then we can use Moab's AdaptiveKDTree to find closest points.
    const smtk::mesh::moab::InterfacePtr& interface =
      std::static_pointer_cast<smtk::mesh::moab::Interface>(meshset.resource()->interface());

    auto search = pointLocatorCache.m_caches.find(meshset.id());
    if (search == pointLocatorCache.m_caches.end())
    {
      // This option restricts the KD tree from subdividing too much
      ::moab::FileOptions treeOptions("MAX_DEPTH=13");

      search =
        pointLocatorCache.m_caches
          .emplace(
            meshset.id(),
            std::unique_ptr<PointLocatorCache::CacheForIndex>(new PointLocatorCache::CacheForIndex(
              interface->moabInterface(), smtkToMOABRange(meshset.cells().range()), &treeOptions)))
          .first;
    }

    ::moab::EntityHandle triangleOut;

    // Identify the nearest point and the associated triangle
    // cacheForIndex->m_tree.closest_triangle(
    //   cacheForIndex->m_treeRootSet, point.data(), &returnValue.second[0], triangleOut);
    search->second->m_tree.closest_triangle(
      search->second->m_treeRootSet, point.data(), returnValue.second.data(), triangleOut);

    returnValue.first = 0.;
    for (int i = 0; i < 3; i++)
    {
      returnValue.first += (point[i] - returnValue.second[i]) * (point[i] - returnValue.second[i]);
    }
    returnValue.first = sqrt(returnValue.first);
  }

  return returnValue;
}
} // namespace moab
} // namespace mesh
} // namespace smtk
