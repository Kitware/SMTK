//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "InverseDistanceWeighting.h"

#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"

#include <cmath>
#include <utility>

namespace
{
// We use inverse distance weighting via Shepard's method, implmented below.
const double EPSILON = 1.e-10;

double euclideanDistance(const std::array<double, 3>& p1, const std::array<double, 3>& p2)
{
  std::array<double, 3> diff = { { p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] } };
  return std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
}

class InverseDistanceWeightingForPointCloud
{
public:
  InverseDistanceWeightingForPointCloud(
    const smtk::mesh::PointCloud& pointcloud,
    double power,
    std::function<bool(double)> prefilter)
    : m_pointcloud(pointcloud)
    , m_power(power)
    , m_prefilter(prefilter)
  {
  }

  // Return the interpolated value at <p> as a weighted sum of the sources
  double operator()(const std::array<double, 3>& p) const
  {
    double d = 0., w = 0., num = 0., denom = 0.;
    for (std::size_t i = 0; i < m_pointcloud.size(); i++)
    {
      if (m_pointcloud.containsIndex(i))
      {
        d = euclideanDistance(p, m_pointcloud.coordinates()(i));
        // If d is zero, then return the value associated with the source point.
        if (d < EPSILON)
        {
          return m_pointcloud.data()(i);
        }
        // Otherwise, sum the contribution from each point.
        w = std::pow(d, -1. * m_power);
        num += w * m_pointcloud.data()(i);
        denom += w;
      }
    }

    return num / denom;
  }

private:
  const smtk::mesh::PointCloud m_pointcloud;
  double m_power;
  std::function<bool(double)> m_prefilter;
};

class InverseDistanceWeightingForStructuredGrid
{
public:
  InverseDistanceWeightingForStructuredGrid(
    const smtk::mesh::StructuredGrid& structuredgrid,
    double power,
    std::function<bool(double)> prefilter)
    : m_structuredgrid(structuredgrid)
    , m_power(power)
    , m_prefilter(prefilter)
  {
  }

  // Return the interpolated value at <p> as a weighted sum of the sources
  double operator()(const std::array<double, 3>& p) const
  {
    double d = 0., w = 0., num = 0., denom = 0.;
    for (int i = m_structuredgrid.m_extent[0]; i < m_structuredgrid.m_extent[1]; i++)
    {
      for (int j = m_structuredgrid.m_extent[2]; j < m_structuredgrid.m_extent[3]; j++)
      {
        if (m_structuredgrid.containsIndex(i, j))
        {
          std::array<double, 3> pij = {
            { (m_structuredgrid.m_origin[0] +
               (i - m_structuredgrid.m_extent[0]) * m_structuredgrid.m_spacing[0]),
              (m_structuredgrid.m_origin[1] +
               (j - m_structuredgrid.m_extent[2]) * m_structuredgrid.m_spacing[1]),
              0. }
          };

          double value = m_structuredgrid.data()(i, j);
          if (m_prefilter(value))
          {
            d = euclideanDistance(p, pij);
            // If d is zero, then return the value associated with the source point.
            if (d < EPSILON)
            {
              return value;
            }
            // Otherwise, sum the contribution from each point.
            w = std::pow(d, -1. * m_power);
            num += w * value;
            denom += w;
          }
        }
      }
    }

    return num / denom;
  }

private:
  const smtk::mesh::StructuredGrid m_structuredgrid;
  double m_power;
  std::function<bool(double)> m_prefilter;
};
} // namespace

namespace smtk
{
namespace mesh
{

InverseDistanceWeighting::InverseDistanceWeighting(
  const PointCloud& pointcloud,
  double power,
  std::function<bool(double)> prefilter)
  : m_function(InverseDistanceWeightingForPointCloud(pointcloud, power, prefilter))
{
}

InverseDistanceWeighting::InverseDistanceWeighting(
  const StructuredGrid& structuredgrid,
  double power,
  std::function<bool(double)> prefilter)
  : m_function(InverseDistanceWeightingForStructuredGrid(structuredgrid, power, prefilter))
{
}
} // namespace mesh
} // namespace smtk
