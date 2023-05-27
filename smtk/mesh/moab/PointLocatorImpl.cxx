//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/moab/PointLocatorImpl.h"
#include "moab/FileOptions.hpp"
#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/Interface.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
#include "moab/ReadUtilIface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <algorithm>
#include <array>

namespace
{
smtk::mesh::Handle create_point_mesh(
  ::moab::Interface* iface,
  std::size_t numPoints,
  const std::function<std::array<double, 3>(std::size_t)>& coordinates,
  ::moab::Range& points)
{
  ::moab::ReadUtilIface* alloc;
  iface->query_interface(alloc);

  //allocate enough space for out points
  smtk::mesh::Handle firstId;
  std::vector<double*> coords;
  alloc->get_node_coords(
    3, //x,y,z
    static_cast<int>(numPoints),
    0, //preferred_start_id
    firstId,
    coords);

  //copy the points into the resource
  for (std::size_t i = 0; i < numPoints; ++i)
  {
    std::array<double, 3> x = coordinates(i);
    for (std::size_t j = 0; j < 3; ++j)
    {
      coords[j][i] = x[j];
    }
  }
  points.insert(firstId, firstId + numPoints - 1);

  smtk::mesh::Handle meshHandle;
  const unsigned int options = 0;
  ::moab::ErrorCode rval = iface->create_meshset(options, meshHandle);
  if (rval == ::moab::MB_SUCCESS)
  {
    iface->add_entities(meshHandle, points);
    iface->add_parent_child(iface->get_root_set(), meshHandle);
  }
  return meshHandle;
}
} // namespace

namespace smtk
{
namespace mesh
{
namespace moab
{

PointLocatorImpl::PointLocatorImpl(::moab::Interface* interface, const ::moab::Range& points)
  : m_interface(interface)
  , m_meshOwningPoints()
  , m_deletePoints(false)
  , m_tree(interface, points)
{
}

PointLocatorImpl::PointLocatorImpl(
  ::moab::Interface* interface,
  std::size_t numPoints,
  const std::function<std::array<double, 3>(std::size_t)>& coordinates)
  : m_interface(interface)
  , m_meshOwningPoints()
  , m_deletePoints(true)
  , m_tree(interface)
{
  ::moab::Range points;
  m_meshOwningPoints =
    create_point_mesh(interface, static_cast<int>(numPoints), coordinates, points);
  // hacker solution to speed up the bathymetry Operation
  ::moab::FileOptions treeOptions("MAX_DEPTH=13");
  m_tree.build_tree(points, nullptr, &treeOptions);
}

PointLocatorImpl::~PointLocatorImpl()
{
  m_tree.reset_tree();
  if (m_deletePoints)
  {
    //we don't delete the vertices, as those can't be explicitly deleted
    //instead they are deleted when the mesh goes away
    m_interface->delete_entities(&m_meshOwningPoints, 1);
  }
}

smtk::mesh::HandleRange PointLocatorImpl::range() const
{
  ::moab::Range entities;
  m_interface->get_entities_by_handle(m_meshOwningPoints, entities);
  return moabToSMTKRange(entities);
}

namespace
{

template<bool>
void reserve_space(std::vector<double>& container, std::size_t size)
{
  container.reserve(size);
}

template<>
void reserve_space<false>(std::vector<double>& /*unused*/, std::size_t /*unused*/)
{
}

template<bool>
void add_to(std::vector<double>& container, double value)
{
  container.push_back(value);
}

template<>
void add_to<false>(std::vector<double>& /*unused*/, double /*unused*/)
{
}

template<bool SaveSqDistances, bool SaveCoords>
void find_valid_points(
  const double x,
  const double y,
  const double z,
  const double sqRadius,
  const ::moab::Range& points,
  const std::vector<double>& x_locs,
  const std::vector<double>& y_locs,
  const std::vector<double>& z_locs,
  std::size_t pointIdOffset,
  smtk::mesh::PointLocatorImpl::Results& results)
{

  const std::size_t numPoints = points.size();

  ::moab::Range::const_iterator ptIter = points.begin();
  std::vector<smtk::mesh::Handle> ptId_temp;

  //clear any existing data from the arrays
  results.pointIds.clear();
  results.sqDistances.clear();
  results.x_s.clear();
  results.y_s.clear();
  results.z_s.clear();

  results.pointIds.reserve(numPoints);
  reserve_space<SaveSqDistances>(results.sqDistances, numPoints);
  reserve_space<SaveCoords>(results.x_s, numPoints);
  reserve_space<SaveCoords>(results.y_s, numPoints);
  reserve_space<SaveCoords>(results.z_s, numPoints);

  for (std::size_t i = 0; i < numPoints; ++i, ++ptIter)
  {
    const double sqLen = (x - x_locs[i]) * (x - x_locs[i]) + (y - y_locs[i]) * (y - y_locs[i]) +
      (z - z_locs[i]) * (z - z_locs[i]);

    if (sqLen <= sqRadius)
    {
      results.pointIds.push_back(static_cast<std::size_t>(*ptIter) - pointIdOffset);
      add_to<SaveSqDistances>(results.sqDistances, sqLen);
      add_to<SaveCoords>(results.x_s, x_locs[i]);
      add_to<SaveCoords>(results.y_s, y_locs[i]);
      add_to<SaveCoords>(results.z_s, z_locs[i]);
    }
  }
}
} // namespace

void PointLocatorImpl::locatePointsWithinRadius(
  double x,
  double y,
  double z,
  double radius,
  Results& results)
{
  std::vector<::moab::EntityHandle> leaves;
  double xyz[3] = { x, y, z };
  m_tree.distance_search(xyz, radius, leaves);

  if (leaves.empty())
  {
    return;
  }

  //now we need to get all the points from the leaves, and do a finer
  //comparison on which ones are within distance of the input point
  ::moab::Range points;
  for (std::size_t i = 0; i < leaves.size(); ++i)
  {
    m_interface->get_entities_by_dimension(leaves[i], 0, points);
  }

  const double sqRadius = radius * radius;
  const std::size_t numPoints = points.size();
  std::vector<double> x_locs(numPoints), y_locs(numPoints), z_locs(numPoints);
  m_interface->get_coords(points, x_locs.data(), y_locs.data(), z_locs.data());

  //now iterate and compute distances. We use a templated version of
  //find_valid_points so that we generate 4 versions of the algorithm that
  //each are optimal for what the caller wants saved. This means that we
  //don't have to take 2 extra branches inside the tight loop
  const bool wantDistance = results.want_sqDistances;
  const bool wantCoords = results.want_Coordinates;

  ::moab::Range entities;
  m_interface->get_entities_by_handle(m_meshOwningPoints, entities);
  ::moab::EntityHandle firstCell = entities[0];

  if (wantDistance && wantCoords)
  {
    find_valid_points<true, true>(
      x, y, z, sqRadius, points, x_locs, y_locs, z_locs, firstCell, results);
  }
  else if (wantDistance)
  {
    find_valid_points<true, false>(
      x, y, z, sqRadius, points, x_locs, y_locs, z_locs, firstCell, results);
  }
  else if (wantCoords)
  {
    find_valid_points<false, true>(
      x, y, z, sqRadius, points, x_locs, y_locs, z_locs, firstCell, results);
  }
  else
  {
    find_valid_points<false, false>(
      x, y, z, sqRadius, points, x_locs, y_locs, z_locs, firstCell, results);
  }
}
} // namespace moab
} // namespace mesh
} // namespace smtk
