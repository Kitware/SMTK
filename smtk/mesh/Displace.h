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

  //This operation is the mesh mirror for model bathymetry operations.
  //Given a set of mesh elements and either a point cloud or a structured grid
  //of elevation points, displace the mesh z values using the input data as a
  //reference.
  class ElevationStructuredData
  {
  public:
    // Given indices int othe structured data, determine whether or not the cell
    // is blanked.
    virtual double operator()(int, int) const = 0;
    virtual bool containsIndex(int ix, int iy) const
      {
        return (ix >= m_extent[0] && ix <= m_extent[1] &&
                iy >= m_extent[2] && iy <= m_extent[3]);
      }


    int m_extent[4]; // indices for xmin, xmax, ymin, ymax
    double m_bounds[4]; // xmin, xmax, ymin, ymax
  };

  class ElevationControls
  {
    public:
    ElevationControls():
      m_clampMin(false),
      m_clampMax(false),
      m_useInvalid(false)
    {
    }

    ElevationControls(bool clampMin, double minElev,
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

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const std::vector<double>& pointcloud,
                const smtk::mesh::MeshSet& ms,
                double radius,
                ElevationControls controls = ElevationControls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const std::vector<double>& pointcloud,
                const smtk::mesh::PointSet& ps,
                double radius,
                ElevationControls controls = ElevationControls() );


#ifndef SHIBOKEN_SKIP
  // Skipping the following:
  // double*, and float* they are not nicely wrapped so the length is unknown
  // std::vector<float>& skipped since python uses doubles not floats

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const double* const pointcloud, std::size_t numPoints,
                const smtk::mesh::MeshSet& ms,
                double radius,
                ElevationControls controls = ElevationControls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const float* const pointcloud, std::size_t numPoints,
                const smtk::mesh::MeshSet& ms,
                double radius,
                ElevationControls controls  = ElevationControls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const double* const pointcloud, std::size_t numPoints,
                const smtk::mesh::PointSet& ps,
                double radius,
                ElevationControls controls = ElevationControls() );

  //flattens point cloud so we have a cylindrical search space
  SMTKCORE_EXPORT
  bool elevate( const float* const pointcloud, std::size_t numPoints,
                const smtk::mesh::PointSet& ps,
                double radius,
                ElevationControls controls  = ElevationControls() );

  SMTKCORE_EXPORT
  bool elevate( const smtk::mesh::ElevationStructuredData& data,
                const smtk::mesh::MeshSet& ms,
                double radius,
                ElevationControls controls = ElevationControls() );

  SMTKCORE_EXPORT
  bool elevate( const smtk::mesh::ElevationStructuredData& data,
                const smtk::mesh::PointSet& ps,
                double radius,
                ElevationControls controls = ElevationControls() );

#endif

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

  }
}

#endif
