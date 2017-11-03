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

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/core/PointLocator.h"
#include "smtk/mesh/core/PointSet.h"

#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"

#include <cmath>
#include <utility>

namespace
{
struct RadialAverageForPointCloud
{
  RadialAverageForPointCloud(
    smtk::mesh::CollectionPtr collection, const smtk::mesh::PointCloud& pointcloud, double radius)
    : m_pointcloud(pointcloud)
    , m_radius(radius)
    , m_locator(collection, pointcloud.size(), pointcloud.coordinates())
  {
  }

  double operator()(std::array<double, 3> x)
  {
    smtk::mesh::PointLocator::LocatorResults results;
    m_locator.find(x[0], x[1], 0.0, m_radius, results);

    if (results.pointIds.empty())
    {
      return std::numeric_limits<double>::quiet_NaN();
    }

    //otherwise we need to average the z values
    double sum = 0;
    std::size_t numPointsInRadius = 0;
    for (auto i : results.pointIds)
    {
      sum += m_pointcloud.data()(i);
      ++numPointsInRadius;
    }
    return sum / numPointsInRadius;
  }

  const smtk::mesh::PointCloud m_pointcloud;
  double m_radius;
  smtk::mesh::PointLocator m_locator;
};

struct RadialAverageForStructuredGrid
{
  typedef std::pair<int, int> Coord;

  RadialAverageForStructuredGrid(const smtk::mesh::StructuredGrid& structuredgrid, double radius)
    : m_structuredgrid(structuredgrid)
    , m_radius2(radius * radius)
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

  void find(double x, double y, std::vector<Coord>& results) const
  {
    // (ix,iy) represents the closest point in the grid to the query point
    int ix = static_cast<int>(std::round(m_structuredgrid.m_extent[0] +
      ((x - m_structuredgrid.m_origin[0]) / m_structuredgrid.m_spacing[0])));
    int iy = static_cast<int>(std::round(m_structuredgrid.m_extent[2] +
      ((y - m_structuredgrid.m_origin[1]) / m_structuredgrid.m_spacing[1])));

    if (m_discreteRadius[0] == 0 || m_discreteRadius[1] == 0)
    {
      if (m_structuredgrid.containsIndex(ix, iy))
      {
        results.push_back(std::make_pair(ix, iy));
      }
    }
    else
    {
      for (int i = ix - m_discreteRadius[0]; i < ix + m_discreteRadius[0]; i++)
      {
        if (i < m_structuredgrid.m_extent[0] || i > m_structuredgrid.m_extent[1])
        {
          continue;
        }

        // We find the extrema in the y dimension. Every point in between the
        // two extrema will also be in the circle of interest.
        int j_extrema[2] = { iy - m_discreteRadius[1], iy + m_discreteRadius[1] };
        bool extremaFound[2] = { false, false };
        while ((!extremaFound[0] || !extremaFound[1]) && j_extrema[0] != j_extrema[1])
        {
          for (int jdx = 0; jdx < 2; jdx++)
          {
            if (!extremaFound[jdx])
            {
              double x_ = m_structuredgrid.m_origin[0] +
                (i - m_structuredgrid.m_extent[0]) * m_structuredgrid.m_spacing[0];
              double y_ = m_structuredgrid.m_origin[1] +
                (j_extrema[0] - m_structuredgrid.m_extent[2]) * m_structuredgrid.m_spacing[1];

              if (m_limits[0] <= x_ && x_ <= m_limits[1] && m_limits[2] <= y_ && y_ <= m_limits[3])
              {
                double r2 = (x - x_) * (x - x_) + (y - y_) * (y - y_);
                if (r2 < m_radius2)
                {
                  extremaFound[jdx] = true;
                }
              }

              if (!extremaFound[jdx])
              {
                j_extrema[jdx]++;
              }
            }
          }
        }
        for (int j = j_extrema[0]; j < j_extrema[1]; j++)
        {
          if (m_structuredgrid.containsIndex(i, j))
          {
            results.push_back(std::make_pair(i, j));
          }
        }
      }
    }
  }

  double operator()(std::array<double, 3> x)
  {
    if (x[0] < m_limits[0] || x[0] > m_limits[1] || x[1] < m_limits[2] || x[1] > m_limits[3])
    {
      // this point is outside of our data range
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Grab the indices of the points that are within the radius of the
    // point
    std::vector<Coord> results;
    find(x[0], x[1], results);

    if (results.size() == 0)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }

    // We perform an unweighted average to maintain parity with the
    // unstructured grid version of this operator
    double sum = 0.;
    for (std::size_t j = 0; j < results.size(); j++)
    {
      sum += m_structuredgrid.data()(results[j].first, results[j].second);
    }
    return sum / results.size();
  }

  const smtk::mesh::StructuredGrid m_structuredgrid;
  double m_radius2;
  int m_discreteRadius[2];
  double m_limits[4];
};
}

namespace smtk
{
namespace mesh
{

RadialAverage::RadialAverage(
  smtk::mesh::CollectionPtr collection, const PointCloud& pointcloud, double radius)
  : m_function(RadialAverageForPointCloud(collection, pointcloud, radius))
{
}

RadialAverage::RadialAverage(const StructuredGrid& structuredgrid, double radius)
  : m_function(RadialAverageForStructuredGrid(structuredgrid, radius))
{
}
}
}
