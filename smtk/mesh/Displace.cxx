//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "Displace.h"

#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/PointLocator.h"
#include "smtk/mesh/PointSet.h"

namespace
{
  //----------------------------------------------------------------------------
  template<typename T>
  void copy_z_values( const T* const pointcloud,
                      std::size_t numPoints,
                      std::vector<T>& z_values )
  {
  z_values.resize(numPoints);
  for( std::size_t i=0; i < numPoints; ++i)
    {
    z_values[i] = pointcloud[(i*3)+2];
    }
  }

  //----------------------------------------------------------------------------
  template<bool ClampMin, bool ClampMax, typename T>
  struct clamper
  {
    T operator()(T value,  double c_min, double c_max) const
    {
      if(c_min > value)
        { return c_min; }
      if(c_max < value)
        { return c_max; }
      return value;
    }
  };

  template<typename T>
  struct clamper<true,false, T>
  {
    T operator()(T value,  double c_min, double) const
    {
      if (c_min > value)
        { return c_min; }
      return value;
    }
  };

  template<typename T>
  struct clamper<false,true, T>
  {
    T operator()(T value,  double, double c_max) const
    {
      if(c_max < value)
        { return c_max; }
      return value;
    }
  };

  template<typename T>
  struct clamper<false,false, T>
  {
    T operator()(T value,  double, double) const
    {
      return value;
    }
  };

  //----------------------------------------------------------------------------
  template<bool ClampMin, bool ClampMax, typename T>
  void clamp_z_values(std::vector<T>& z_values, double c_min, double c_max)
  {
    clamper<ClampMin, ClampMax, T> clamp;
    std::size_t numPoints = z_values.size();
    for(std::size_t i=0; i < numPoints; ++i)
      {
      z_values[i] = clamp(z_values[i], c_min, c_max);
      }
  }

  //----------------------------------------------------------------------------
  template<typename T>
  class ElevatePoint : public smtk::mesh::PointForEach
  {
    smtk::mesh::PointLocator& m_locator;
    std::vector<T>& m_zValues;
     //represents the value to convert pointId's into m_zValues offsets
    std::size_t m_pointIdOffset;
    double m_radius;
    bool m_useInvalid;
    double m_invalid;
  public:
    ElevatePoint(smtk::mesh::PointLocator& locator,
                 std::vector<T>& z_values,
                 double radius,
                 const smtk::mesh::elevation::clamp_controls& controls):
      m_locator(locator),
      m_zValues(z_values),
      m_pointIdOffset( locator.range()[0] ),
      m_radius(radius),
      m_useInvalid(controls.m_useInvalid),
      m_invalid(controls.m_invalid)
    {
    }

    void forPoints(const smtk::mesh::HandleRange& pointIds,
                   std::vector<double>& xyz,
                   bool& coordinatesModified)
    {
    smtk::mesh::PointLocator::LocatorResults results;

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0.0;
    for(c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset+=3)
      {
      m_locator.find( xyz[offset], xyz[offset+1], 0.0, m_radius, results);

      if(results.pointIds.empty() && m_useInvalid)
        { //this point had nothing near it. Elevate it to
        xyz[offset+2] = m_invalid;
        continue;
        }

      //otherwise we need to average the z values
      T sum = 0;
      std::size_t numPointsInRadius = 0;
      for( c_it j=results.pointIds.begin(); j!=results.pointIds.end(); ++j)
        {
        //current issue, need to determine the offset
        const std::size_t z_index = static_cast<std::size_t>(*j) - m_pointIdOffset;
        sum += m_zValues[ z_index  ];
        ++numPointsInRadius;
        }
      xyz[offset+2] = static_cast<double>( (sum/numPointsInRadius) );
      }
    coordinatesModified = true; //mark we are going to modify the points
    }
  };

  //----------------------------------------------------------------------------
  template<typename T>
  bool do_elevate( const T* const pointcloud,
                   std::size_t numPoints,
                   const smtk::mesh::PointSet& ps,
                   double radius,
                   smtk::mesh::elevation::clamp_controls controls)
  {
  if(pointcloud == NULL || numPoints == 0)
    { //can't elevate with an empty point cloud
    return false;
    }

  //step 1. Copy the z values out of point cloud into a separate array
  //and make a copy
  std::vector<T> z_values;
  copy_z_values(pointcloud, numPoints, z_values);

  //step 2. Clamp if required the z value we just copied out
  if(controls.m_clampMin && controls.m_clampMax)
    {
    clamp_z_values<true,true>(z_values, controls.m_minElev, controls.m_maxElev);
    }
  else if(controls.m_clampMin)
    {
    clamp_z_values<true,false>(z_values, controls.m_minElev, controls.m_maxElev);
    }
  else if(controls.m_clampMax)
    {
    clamp_z_values<false,true>(z_values, controls.m_minElev, controls.m_maxElev);
    }

  //step 3. Create the point locator inputting the flat cloud
  const bool ignoreZValues = true;
  smtk::mesh::PointLocator locator(ps.collection(), pointcloud, numPoints, ignoreZValues);

  //step 4. now elevate each point
  ElevatePoint<T> functor(locator, z_values, radius, controls);
  smtk::mesh::for_each(ps, functor);

  return true;
  }
}

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
bool elevate( const double* const pointcloud,
              std::size_t numPoints,
              const smtk::mesh::MeshSet& ms,
              double radius,
              elevation::clamp_controls controls )
{
  return do_elevate(pointcloud, numPoints, ms.points(), radius, controls);
}

//----------------------------------------------------------------------------
bool elevate( const float* const pointcloud,
              std::size_t numPoints,
              const smtk::mesh::MeshSet& ms,
              double radius,
              elevation::clamp_controls controls )
{
  return do_elevate(pointcloud, numPoints, ms.points(), radius, controls);
}

//----------------------------------------------------------------------------
bool elevate( const double* const pointcloud,
              std::size_t numPoints,
              const smtk::mesh::PointSet& ps,
              double radius,
              elevation::clamp_controls controls )
{
  return do_elevate(pointcloud, numPoints, ps, radius, controls);
}

//----------------------------------------------------------------------------
bool elevate( const float* const pointcloud,
              std::size_t numPoints,
              const smtk::mesh::PointSet& ps,
              double radius,
              elevation::clamp_controls controls )
{
  return do_elevate(pointcloud, numPoints, ps, radius, controls);
}

namespace
{
  //----------------------------------------------------------------------------
  class DisplacePoint : public smtk::mesh::PointForEach
  {
    smtk::mesh::PointLocator& m_locator;
    double m_radius;
  public:
    DisplacePoint(smtk::mesh::PointLocator& locator,
                  double radius):
      m_locator(locator),
      m_radius(radius)
    {
    }

    void forPoints(const smtk::mesh::HandleRange& pointIds,
                   std::vector<double>& xyz,
                   bool& coordinatesModified)
    {
    smtk::mesh::PointLocator::LocatorResults results;
    results.want_Coordinates = true;

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for(c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset+=3)
      {
      m_locator.find( xyz[offset], xyz[offset+1], xyz[offset+2], m_radius, results);

      if(!results.pointIds.empty())
        {
        double sums[3] = {0.0, 0.0, 0.0};
        const std::size_t numPointsInRadius = results.x_s.size();
        for( std::size_t j=0; j < numPointsInRadius; ++j)
          {
          sums[0] += results.x_s[j];
          sums[1] += results.y_s[j];
          sums[2] += results.z_s[j];
          }
        xyz[offset] = (sums[0]/numPointsInRadius);
        xyz[offset+1] = (sums[1]/numPointsInRadius);
        xyz[offset+2] = (sums[2]/numPointsInRadius);
        }
      }
    coordinatesModified = true; //mark we are going to modify the points
    }
  };
}

//----------------------------------------------------------------------------
bool displace( const smtk::mesh::PointSet& pointcloud,
               const smtk::mesh::MeshSet& ms,
               double radius)
{
  smtk::mesh::PointLocator locator(pointcloud);

  DisplacePoint functor(locator, radius);
  smtk::mesh::for_each(ms.points(), functor);
  return true;
}

//----------------------------------------------------------------------------
bool displace( const smtk::mesh::PointSet& pointcloud,
               const smtk::mesh::PointSet& ps,
               double radius)
{
  smtk::mesh::PointLocator locator(pointcloud);

  DisplacePoint functor(locator, radius);
  smtk::mesh::for_each(ps, functor);
  return true;
}

}
}