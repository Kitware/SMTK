//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Displace_h
#define __smtk_mesh_Displace_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/MeshSet.h"

namespace smtk {
  namespace mesh {

#ifndef SHIBOKEN_SKIP
  //This operation is the mesh mirror for model bathymetry operations.
  //Given a set of mesh elements, and a point cloud. Displace the mesh
  //z values using the point cloud as a reference

  //Things we need to handle
  // clamping for both Low and High
  // Invalid value ( )
  // Radius
  //Options:
  // Looks like the classic implementation flattens both the mesh and point
  // cloud down a z value of zero. Than it caches the clamped point cloud
  // z values in a separate array.

  namespace elevation {
    struct clamp_controls {

    clamp_controls():
      m_clampMin(false),
      m_clampMax(false),
      m_useInvalid(false)
    {
    }

    clamp_controls(bool clampMin, double minElev,
                   bool clampMax, double maxElev,
                   bool useInvalid=false, double invalid=0.0):
      m_clampMin(clampMin),
      m_clampMax(clampMax),
      m_useInvalid(useInvalid),
      m_minElev(minElev),
      m_maxElev(maxElev),
      m_invalid(invalid)
      {
      }

    bool m_clampMin;
    bool m_clampMax;
    bool m_useInvalid;
    double m_minElev;
    double m_maxElev;
    double m_invalid;
  };
  }

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const double* const pointcloud, std::size_t numPoints,
                const smtk::mesh::MeshSet& ms,
                double radius,
                elevation::clamp_controls controls = elevation::clamp_controls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const float* const pointcloud, std::size_t numPoints,
                const smtk::mesh::MeshSet& ms,
                double radius,
                elevation::clamp_controls controls  = elevation::clamp_controls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const double* const pointcloud, std::size_t numPoints,
                const smtk::mesh::PointSet& ps,
                double radius,
                elevation::clamp_controls controls = elevation::clamp_controls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const float* const pointcloud, std::size_t numPoints,
                const smtk::mesh::PointSet& ps,
                double radius,
                elevation::clamp_controls controls  = elevation::clamp_controls() );

  //displace a set of points, given a point cloud. Doesn't flatten like
  //elevate. If multiple points from the cloud are within the radius the
  //displacement is equal to the centroid of the points
  SMTKCORE_EXPORT
  bool displace( const smtk::mesh::PointSet& pointcloud,
                 const smtk::mesh::MeshSet& ms,
                 double radius);

  //displace a set of points, given a point cloud. Doesn't flatten like
  //elevate. If multiple points from the cloud are within the radius the
  //displacement is equal to the centroid of the points
  SMTKCORE_EXPORT
  bool displace( const smtk::mesh::PointSet& pointcloud,
                 const smtk::mesh::PointSet&,
                 double radius);



#endif
  }
}

#endif
