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
#include "smtk/mesh/moab/Interface.h"

#include "moab/AdaptiveKDTree.hpp"
#include "moab/ReadUtilIface.hpp"

#include <algorithm>


//----------------------------------------------------------------------------
namespace {
template<typename T>
smtk::mesh::Handle create_point_mesh(::moab::Interface* iface,
                                     const T* const xyzs,
                                     std::size_t numPoints,
                                     smtk::mesh::HandleRange& points)
{
  ::moab::ReadUtilIface* alloc;
  iface->query_interface(alloc);

  //allocate enough space for out points
  smtk::mesh::Handle firstId;
  std::vector<double* > coords;
  alloc->get_node_coords(3, //x,y,z
                         numPoints,
                         0, //preferred_start_id
                         firstId,
                         coords);

  //copy the points into the collection
  for( std::size_t i=0; i < numPoints; ++i)
    {
    coords[0][i] = static_cast<double>(xyzs[(i*3)]);
    coords[1][i] = static_cast<double>(xyzs[(i*3)+1]);
    coords[2][i] = static_cast<double>(xyzs[(i*3)+2]);
    }
  points.insert(firstId, firstId+numPoints-1);

  smtk::mesh::Handle meshHandle;
  const unsigned int options = 0;
  ::moab::ErrorCode rval = iface->create_meshset( options , meshHandle );
  if(rval == ::moab::MB_SUCCESS)
    {
    iface->add_entities( meshHandle, points );
    iface->add_parent_child( iface->get_root_set(), meshHandle );
    }
  return meshHandle;
}
}

namespace smtk {
namespace mesh {
namespace moab {

//----------------------------------------------------------------------------
PointLocatorImpl::PointLocatorImpl(::moab::Interface* interface,
                                   const smtk::mesh::HandleRange& points):
m_interface( interface ),
m_meshOwningPoints(),
m_deletePoints( false ),
m_tree(interface, points)
{

}

//----------------------------------------------------------------------------
PointLocatorImpl::PointLocatorImpl(::moab::Interface* interface,
                                   const double* const xyzs,
                                   std::size_t numPoints):
m_interface( interface ),
m_meshOwningPoints(),
m_deletePoints( true ),
m_tree(interface)
{
  smtk::mesh::HandleRange points;
  m_meshOwningPoints = create_point_mesh(interface, xyzs, numPoints, points);
  m_tree.build_tree(points);
}

//----------------------------------------------------------------------------
PointLocatorImpl::PointLocatorImpl(::moab::Interface* interface,
                                   const float* const xyzs,
                                   std::size_t numPoints):
m_interface( interface ),
m_meshOwningPoints( ),
m_deletePoints( true ),
m_tree(interface)
{
  smtk::mesh::HandleRange points;
  m_meshOwningPoints = create_point_mesh(interface, xyzs, numPoints, points);
  m_tree.build_tree(points);
}

//----------------------------------------------------------------------------
PointLocatorImpl::~PointLocatorImpl()
{
  m_tree.reset_tree();
  if(m_deletePoints)
    {
    //we don't delete the vertices, as those can't be explicitly deleted
    //instead they are deleted when the mesh goes away
    m_interface->delete_entities( &m_meshOwningPoints, 1 );
    }
}


//----------------------------------------------------------------------------
namespace {

template< bool>
void reserve_space(std::vector<double>& container, std::size_t size) { container.reserve(size); }

template< >
void reserve_space < false > (std::vector<double>& container, std::size_t) { }

template< bool >
void add_to(std::vector<double>& container, double value) { container.push_back(value); }

template< >
void add_to < false > (std::vector<double>& container, double) { }

template< bool SaveSqDistances, bool SaveCoords >
void find_valid_points(const double x, const double y, const double z,
                       const double sqRadius,
                       const smtk::mesh::HandleRange& points,
                       const std::vector<double>& x_locs,
                       const std::vector<double>& y_locs,
                       const std::vector<double>& z_locs,
                       smtk::mesh::PointLocatorImpl::Results& results)
{

  const std::size_t numPoints = points.size();

  smtk::mesh::HandleRange::const_iterator ptIter = points.begin();
  std::vector<smtk::mesh::Handle> ptId_temp;

  //clear any existing data from the arrays
  results.pointIds.clear();
  results.sqDistances.clear();
  results.x_s.clear();
  results.y_s.clear();
  results.z_s.clear();

  ptId_temp.reserve(numPoints);
  reserve_space<SaveSqDistances>(results.sqDistances,numPoints);
  reserve_space<SaveCoords>(results.x_s, numPoints);
  reserve_space<SaveCoords>(results.y_s, numPoints);
  reserve_space<SaveCoords>(results.z_s, numPoints);

  for(std::size_t i=0; i < numPoints; ++i, ++ptIter)
    {
    const double sqLen = (x - x_locs[i]) * (x - x_locs[i]) +
                         (y - y_locs[i]) * (y - y_locs[i]) +
                         (z - z_locs[i]) * (z - z_locs[i]);

    if (sqLen <= sqRadius)
      {
      ptId_temp.push_back( *ptIter );
      add_to<SaveSqDistances>(results.sqDistances,sqLen);
      add_to<SaveCoords>(results.x_s, x_locs[i]);
      add_to<SaveCoords>(results.y_s, y_locs[i]);
      add_to<SaveCoords>(results.z_s, z_locs[i]);
      }
    }

  std::copy( ptId_temp.rbegin(), ptId_temp.rend(), ::moab::range_inserter(results.pointIds) );
}

}

//----------------------------------------------------------------------------
void PointLocatorImpl::locatePointsWithinRadius(double x, double y, double z,
                                                double radius,
                                                Results& results)
{
  std::vector< ::moab::EntityHandle > leaves;
  double xyz[3] = { x, y, z };
  m_tree.distance_search(xyz, radius, leaves);

  if(leaves.empty())
    {
    return;
    }

  //now we need to get all the points from the leaves, and do a finer
  //comparison on which ones are within distance of the input point
  smtk::mesh::HandleRange points;
  for(std::size_t i=0; i < leaves.size(); ++i)
    {
    m_interface->get_entities_by_dimension( leaves[i], 0, points);
    }


  const double sqRadius = radius * radius;
  const std::size_t numPoints = points.size();
  std::vector< double > x_locs(numPoints), y_locs(numPoints), z_locs(numPoints);
  m_interface->get_coords( points, &x_locs[0], &y_locs[0], &z_locs[0] );

  //now iterate and compute distances. We use a templated version of
  //find_valid_points so that we generate 4 versions of the algorithm that
  //each are optimal for what the caller wants saved. This means that we
  //don't have to take 2 extra branches inside the tight loop
  const bool wantDistance = results.want_sqDistances;
  const bool wantCoords = results.want_Coordinates;
  if(wantDistance && wantCoords)
    {
    find_valid_points<true,true>(x, y, z,
                                 sqRadius, points,
                                 x_locs, y_locs, z_locs,
                                 results);
    }
  else if(wantDistance)
    {
    find_valid_points<true,false>(x, y, z,
                                  sqRadius, points,
                                  x_locs, y_locs, z_locs,
                                  results);
    }
  else if(wantCoords)
    {
    find_valid_points<false,true>(x, y, z,
                                  sqRadius, points,
                                  x_locs, y_locs, z_locs,
                                  results);
    }
  else
    {
    find_valid_points<false,false>(x, y, z,
     sqRadius, points,
                                   x_locs, y_locs, z_locs,
                                   results);
    }
}

}
}
}
