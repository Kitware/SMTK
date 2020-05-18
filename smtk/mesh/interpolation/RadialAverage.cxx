//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "RadialAverage.h"

#include "smtk/mesh/core/PointLocator.h"
#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"

#include <cmath>
#include <utility>

namespace
{
struct RadialAverageForPointCloud
{
  RadialAverageForPointCloud(
    smtk::mesh::ResourcePtr resource,
    const smtk::mesh::PointCloud& pointcloud,
    double radius,
    std::function<bool(double)> prefilter)
    : m_pointcloud(pointcloud)
    , m_radius(radius)
    , m_prefilter(prefilter)
    , m_locator(resource, pointcloud.size(), pointcloud.coordinates())
  {
  }

  double operator()(std::array<double, 3> x)
  {
    smtk::mesh::PointLocator::LocatorResults results;
    m_locator.find(x[0], x[1], 0.0, m_radius, results);

    double sum = 0;
    std::size_t numPointsInRadius = 0;
    for (auto i : results.pointIds)
    {
      if (m_pointcloud.containsIndex(i))
      {
        double value = m_pointcloud.data()(i);
        if (m_prefilter(value))
        {
          sum += value;
          ++numPointsInRadius;
        }
      }
    }

    if (numPointsInRadius == 0)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      return sum / numPointsInRadius;
    }
  }

  const smtk::mesh::PointCloud m_pointcloud;
  double m_radius;
  std::function<bool(double)> m_prefilter;
  smtk::mesh::PointLocator m_locator;
};

struct RadialAverageForStructuredGrid
{
  typedef std::pair<int, int> Coord;

  RadialAverageForStructuredGrid(
    const smtk::mesh::StructuredGrid& structuredgrid,
    double radius,
    std::function<bool(double)> prefilter)
    : m_structuredgrid(structuredgrid)
    , m_radius2(radius * radius)
    , m_prefilter(prefilter)
  {
    m_discreteRadius[0] =
      std::abs(static_cast<int>(std::round(radius / m_structuredgrid.m_spacing[0])));
    m_discreteRadius[1] =
      std::abs(static_cast<int>(std::round(radius / m_structuredgrid.m_spacing[1])));

    m_limits[0] = m_structuredgrid.m_origin[0];
    m_limits[1] = m_structuredgrid.m_origin[0] +
      (m_structuredgrid.m_extent[1] - m_structuredgrid.m_extent[0]) * m_structuredgrid.m_spacing[0];
    if (m_limits[0] > m_limits[1])
    {
      std::swap(m_limits[0], m_limits[1]);
    }
    m_limits[2] = m_structuredgrid.m_origin[1];
    m_limits[3] = m_structuredgrid.m_origin[1] +
      (m_structuredgrid.m_extent[3] - m_structuredgrid.m_extent[2]) * m_structuredgrid.m_spacing[1];
    if (m_limits[2] > m_limits[3])
    {
      std::swap(m_limits[2], m_limits[3]);
    }
  }

  double operator()(std::array<double, 3> x)
  {
    if (x[0] < m_limits[0] || x[0] > m_limits[1] || x[1] < m_limits[2] || x[1] > m_limits[3])
    {
      // this point is outside of our data range
      return std::numeric_limits<double>::quiet_NaN();
    }

    // (ix,iy) represents the closest point in the grid to the query point
    int ix = static_cast<int>(std::round(
      m_structuredgrid.m_extent[0] +
      ((x[0] - m_structuredgrid.m_origin[0]) / m_structuredgrid.m_spacing[0])));
    int iy = static_cast<int>(std::round(
      m_structuredgrid.m_extent[2] +
      ((x[1] - m_structuredgrid.m_origin[1]) / m_structuredgrid.m_spacing[1])));

    // Since we allow for different spacing in x and y, our circle maps to an
    // ellipse with axes in the x and y dimensions of m_discreteRadius[0] and
    // m_discreteRadius[1], respectively, in discrete space. To integrate this
    // ellipse, we sweep across the y dimension; for each discrete step in y,
    // we compute the extrema in x and sum the results at each coordinate.
    double sum = 0.;
    std::size_t nCoords = 0;
    int i_extrema[2];
    for (int j = iy - m_discreteRadius[1]; j < iy + m_discreteRadius[1]; j++)
    {
      if (j < m_structuredgrid.m_extent[2] || j > m_structuredgrid.m_extent[3])
      {
        continue;
      }

      int halfChord = int(m_discreteRadius[0] * sin(acos(double(j - iy) / m_discreteRadius[1])));
      i_extrema[0] = ix - halfChord;
      i_extrema[1] = ix + halfChord;

      if (i_extrema[0] < m_structuredgrid.m_extent[0])
      {
        i_extrema[0] = m_structuredgrid.m_extent[0];
      }
      if (i_extrema[1] > m_structuredgrid.m_extent[1])
      {
        i_extrema[1] = m_structuredgrid.m_extent[1];
      }

      // We perform an unweighted average to maintain parity with the
      // unstructured grid version of this operator
      for (int i = i_extrema[0]; i < i_extrema[1]; i++)
      {
        if (m_structuredgrid.containsIndex(i, j))
        {
          double value = m_structuredgrid.data()(i, j);
          if (m_prefilter(value))
          {
            sum += value;
            nCoords++;
          }
        }
      }
    }
    if (nCoords == 0)
    {
      if (m_structuredgrid.containsIndex(ix, iy))
      {
        sum = m_structuredgrid.data()(ix, iy);
      }
      else
      {
        sum = std::numeric_limits<double>::quiet_NaN();
      }
    }
    else
    {
      sum /= nCoords;
    }

    return sum;
  }

  const smtk::mesh::StructuredGrid m_structuredgrid;
  double m_radius2;
  std::function<bool(double)> m_prefilter;
  int m_discreteRadius[2];
  double m_limits[4];
};
} // namespace

namespace smtk
{
namespace mesh
{

RadialAverage::RadialAverage(
  smtk::mesh::ResourcePtr resource,
  const PointCloud& pointcloud,
  double radius,
  std::function<bool(double)> prefilter)
  : m_function(RadialAverageForPointCloud(resource, pointcloud, radius, prefilter))
{
}

RadialAverage::RadialAverage(
  const StructuredGrid& structuredgrid,
  double radius,
  std::function<bool(double)> prefilter)
  : m_function(RadialAverageForStructuredGrid(structuredgrid, radius, prefilter))
{
}
} // namespace mesh
} // namespace smtk
