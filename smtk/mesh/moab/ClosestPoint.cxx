//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/moab/ClosestPoint.h"

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
std::array<double, 3> ClosestPoint::operator()(
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

std::array<double, 3> ClosestPoint::operator()(
  const smtk::mesh::MeshSet& meshset,
  const std::array<double, 3>& point) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  std::array<double, 3> returnValue{ { nan, nan, nan } };

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
    search->second->m_tree.closest_triangle(
      search->second->m_treeRootSet, point.data(), returnValue.data(), triangleOut);

    // ...access the three vertices of the nearest triangle
    ::moab::Range connectivity;
    interface->moabInterface()->get_connectivity(&triangleOut, 1, connectivity);
    std::array<double, 9> coords;
    interface->moabInterface()->get_coords(connectivity, coords.data());

    // Compute the squared distance betwen the source point and the vertex
    std::array<double, 3> dist2 = { { 0., 0., 0. } };
    for (std::size_t j = 0; j < 3; j++)
    {
      for (std::size_t k = 0; k < 3; k++)
      {
        double tmp = coords[3 * j + k] - point[k];
        dist2[j] += tmp * tmp;
      }
    }

    // Assign the closest point to the coordinates of the closest vertex
    std::size_t index = std::distance(dist2.begin(), std::min_element(dist2.begin(), dist2.end()));
    for (std::size_t j = 0; j < 3; j++)
    {
      returnValue[j] = coords[3 * index + j];
    }
  }

  return returnValue;
}
} // namespace moab
} // namespace mesh
} // namespace smtk
