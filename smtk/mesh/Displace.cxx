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

#include <cmath>
#include <utility>

namespace
{
template <typename T>
void copy_z_values(const T* const pointcloud, std::size_t numPoints, std::vector<T>& z_values)
{
  z_values.resize(numPoints);
  for (std::size_t i = 0; i < numPoints; ++i)
  {
    z_values[i] = pointcloud[(i * 3) + 2];
  }
}

template <bool ClampMin, bool ClampMax, typename T>
struct clamper
{
  T operator()(T value, double c_min, double c_max) const
  {
    if (c_min > value)
    {
      return static_cast<T>(c_min);
    }
    if (c_max < value)
    {
      return static_cast<T>(c_max);
    }
    return value;
  }
};

template <typename T>
struct clamper<true, false, T>
{
  T operator()(T value, double c_min, double) const
  {
    if (c_min > value)
    {
      return static_cast<T>(c_min);
    }
    return value;
  }
};

template <typename T>
struct clamper<false, true, T>
{
  T operator()(T value, double, double c_max) const
  {
    if (c_max < value)
    {
      return static_cast<T>(c_max);
    }
    return value;
  }
};

template <typename T>
struct clamper<false, false, T>
{
  T operator()(T value, double, double) const { return value; }
};

template <bool ClampMin, bool ClampMax, typename T>
void clamp_z_values(std::vector<T>& z_values, double c_min, double c_max)
{
  clamper<ClampMin, ClampMax, T> clamp;
  std::size_t numPoints = z_values.size();
  for (std::size_t i = 0; i < numPoints; ++i)
  {
    z_values[i] = clamp(z_values[i], c_min, c_max);
  }
}

template <typename T>
class ElevatePoint : public smtk::mesh::PointForEach
{
  smtk::mesh::PointLocator& m_locator;
  std::vector<T>& m_zValues;
  //represents the value to convert pointIds into m_zValues offsets
  std::size_t m_pointIdOffset;
  double m_radius;
  bool m_useInvalid;
  double m_invalid;

public:
  ElevatePoint(smtk::mesh::PointLocator& locator, std::vector<T>& z_values, double radius,
    const smtk::mesh::ElevationControls& controls)
    : m_locator(locator)
    , m_zValues(z_values)
    , m_pointIdOffset(locator.range()[0])
    , m_radius(radius)
    , m_useInvalid(controls.m_useInvalid)
    , m_invalid(controls.m_invalid)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    smtk::mesh::PointLocator::LocatorResults results;

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      m_locator.find(xyz[offset], xyz[offset + 1], 0.0, m_radius, results);

      if (results.pointIds.empty() && m_useInvalid)
      { //this point had nothing near it. Elevate it to
        xyz[offset + 2] = m_invalid;
        continue;
      }

      //otherwise we need to average the z values
      T sum = 0;
      std::size_t numPointsInRadius = 0;
      for (c_it j = results.pointIds.begin(); j != results.pointIds.end(); ++j)
      {
        //current issue, need to determine the offset
        const std::size_t z_index = static_cast<std::size_t>(*j) - m_pointIdOffset;
        sum += m_zValues[z_index];
        ++numPointsInRadius;
      }
      xyz[offset + 2] = static_cast<double>((sum / numPointsInRadius));
    }
    coordinatesModified = true; //mark we are going to modify the points
  }
};

template <typename T>
bool do_elevate(const T* const pointcloud, std::size_t numPoints, const smtk::mesh::PointSet& ps,
  double radius, smtk::mesh::ElevationControls controls)
{
  if (pointcloud == NULL || numPoints == 0)
  { //can't elevate with an empty point cloud
    return false;
  }

  //step 1. Copy the z values out of point cloud into a separate array
  //and make a copy
  std::vector<T> z_values;
  copy_z_values(pointcloud, numPoints, z_values);

  //step 2. Clamp if required the z value we just copied out
  if (controls.m_clampMin && controls.m_clampMax)
  {
    clamp_z_values<true, true>(z_values, controls.m_minElev, controls.m_maxElev);
  }
  else if (controls.m_clampMin)
  {
    clamp_z_values<true, false>(z_values, controls.m_minElev, controls.m_maxElev);
  }
  else if (controls.m_clampMax)
  {
    clamp_z_values<false, true>(z_values, controls.m_minElev, controls.m_maxElev);
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

template <bool ClampMin, bool ClampMax>
class ElevatePointForStructuredInput : public smtk::mesh::PointForEach
{
  const smtk::mesh::ElevationStructuredData& m_data;
  double m_radius2;
  bool m_useInvalid;
  double m_invalid;
  double m_minElev;
  double m_maxElev;
  int m_discreteRadius[2];

  typedef std::pair<int, int> Coord;

public:
  ElevatePointForStructuredInput(const smtk::mesh::ElevationStructuredData& data, double radius,
    const smtk::mesh::ElevationControls& controls)
    : m_data(data)
    , m_radius2(radius * radius)
    , m_useInvalid(controls.m_useInvalid)
    , m_invalid(controls.m_invalid)
    , m_minElev(controls.m_minElev)
    , m_maxElev(controls.m_maxElev)
  {
    m_discreteRadius[0] = std::abs(static_cast<int>(std::round(radius / m_data.m_spacing[0])));
    m_discreteRadius[1] = std::abs(static_cast<int>(std::round(radius / m_data.m_spacing[1])));
  }

  void find(double x, double y, std::vector<Coord>& results) const
  {
    // (ix,iy) represents the closest point in the grid to the query point
    int ix = static_cast<int>(
      std::round(m_data.m_extent[0] + ((x - m_data.m_origin[0]) / m_data.m_spacing[0])));
    int iy = static_cast<int>(
      std::round(m_data.m_extent[2] + ((y - m_data.m_origin[1]) / m_data.m_spacing[1])));

    if (m_discreteRadius[0] == 0 || m_discreteRadius[1] == 0)
    {
      if (m_data.containsIndex(ix, iy))
      {
        results.push_back(std::make_pair(ix, iy));
      }
    }
    else
    {
      double xStart = m_data.m_origin[0];
      double xEnd =
        m_data.m_origin[0] + (m_data.m_extent[1] - m_data.m_extent[0]) * m_data.m_spacing[0];
      if (xStart > xEnd)
      {
        std::swap(xStart, xEnd);
      }
      double yStart = m_data.m_origin[1];
      double yEnd =
        m_data.m_origin[1] + (m_data.m_extent[3] - m_data.m_extent[2]) * m_data.m_spacing[1];
      if (yStart > yEnd)
      {
        std::swap(yStart, yEnd);
      }

      for (int i = ix - m_discreteRadius[0]; i < ix + m_discreteRadius[0]; i++)
      {
        if (i < m_data.m_extent[0] || i > m_data.m_extent[1])
        {
          continue;
        }

        // We find the extrema in the y dimension. Every point in between the
        // two extrema will also be in the circle of interest.
        int jmin = iy - m_discreteRadius[1];
        int jmax = iy + m_discreteRadius[1];
        bool extremaFound[2] = { false, false };
        while ((!extremaFound[0] || !extremaFound[1]) && jmin != jmax)
        {
          if (!extremaFound[0])
          {
            double x_ = m_data.m_origin[0] + (i - m_data.m_extent[0]) * m_data.m_spacing[0];
            double y_ = m_data.m_origin[1] + (jmin - m_data.m_extent[2]) * m_data.m_spacing[1];

            if (xStart <= x_ && x_ <= xEnd && yStart <= y_ && y_ <= yEnd)
            {
              double r2 = (x - x_) * (x - x_) + (y - y_) * (y - y_);
              if (r2 < m_radius2)
              {
                extremaFound[0] = true;
              }
            }

            if (!extremaFound[0])
            {
              jmin++;
            }
          }

          if (!extremaFound[1])
          {
            double x_ = m_data.m_origin[0] + (i - m_data.m_extent[0]) * m_data.m_spacing[0];
            double y_ = m_data.m_origin[1] + (jmin - m_data.m_extent[2]) * m_data.m_spacing[1];

            if (xStart <= x_ && x_ <= xEnd && yStart <= y_ && y_ <= yEnd)
            {
              double r2 = (x - x_) * (x - x_) + (y - y_) * (y - y_);
              if (r2 < m_radius2)
              {
                extremaFound[1] = true;
              }
            }

            if (!extremaFound[1])
            {
              jmax--;
            }
          }
        }
        for (int j = jmin; j < jmax; j++)
        {
          if (m_data.containsIndex(i, j))
          {
            results.push_back(std::make_pair(i, j));
          }
        }
      }
    }
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    clamper<ClampMin, ClampMax, double> clamp;

    std::vector<Coord> results;

    double xStart = m_data.m_origin[0];
    double xEnd =
      m_data.m_origin[0] + (m_data.m_extent[1] - m_data.m_extent[0]) * m_data.m_spacing[0];
    if (xStart > xEnd)
    {
      std::swap(xStart, xEnd);
    }
    double yStart = m_data.m_origin[1];
    double yEnd =
      m_data.m_origin[1] + (m_data.m_extent[3] - m_data.m_extent[2]) * m_data.m_spacing[1];
    if (yStart > yEnd)
    {
      std::swap(yStart, yEnd);
    }

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      if (xyz[offset] < xStart || xyz[offset] > xEnd || xyz[offset + 1] < yStart ||
        xyz[offset + 1] > yEnd)
      {
        // this point is outside of our data range
        if (m_useInvalid)
        {
          xyz[offset + 2] = m_invalid;
        }
        continue;
      }

      // Grab the indices of the points that are within the radius of the
      // point
      find(xyz[offset], xyz[offset + 1], results);

      if (results.size() == 0)
      {
        if (m_useInvalid)
        {
          xyz[offset + 2] = m_invalid;
        }
        continue;
      }

      // We perform an unweighted average to maintain parity with the
      // unstructured grid version of this operator
      double sum = 0.;
      for (std::size_t j = 0; j < results.size(); j++)
      {
        sum += clamp(m_data(results[j].first, results[j].second), m_minElev, m_maxElev);
      }
      xyz[offset + 2] = static_cast<double>((sum / results.size()));
      results.clear();
    }
    coordinatesModified = true; //mark we are going to modify the points
  }
};

class ElevatePointForFunctionalInput : public smtk::mesh::PointForEach
{
  const std::function<double(double, double)> m_data;

public:
  ElevatePointForFunctionalInput(const std::function<double(double, double)> data)
    : m_data(data)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      xyz[offset + 2] = m_data(xyz[offset], xyz[offset + 1]);
    }
    coordinatesModified = true; //mark we are going to modify the points
  }
};

namespace smtk
{
namespace mesh
{

bool elevate(const std::vector<double>& pointcloud, const smtk::mesh::MeshSet& ms, double radius,
  ElevationControls controls)
{
  return do_elevate(&pointcloud[0], pointcloud.size(), ms.points(), radius, controls);
}

bool elevate(const std::vector<double>& pointcloud, const smtk::mesh::PointSet& ps, double radius,
  ElevationControls controls)
{
  return do_elevate(&pointcloud[0], pointcloud.size(), ps, radius, controls);
}

bool elevate(const double* const pointcloud, std::size_t numPoints, const smtk::mesh::MeshSet& ms,
  double radius, ElevationControls controls)
{
  return do_elevate(pointcloud, numPoints, ms.points(), radius, controls);
}

bool elevate(const float* const pointcloud, std::size_t numPoints, const smtk::mesh::MeshSet& ms,
  double radius, ElevationControls controls)
{
  return do_elevate(pointcloud, numPoints, ms.points(), radius, controls);
}

bool elevate(const double* const pointcloud, std::size_t numPoints, const smtk::mesh::PointSet& ps,
  double radius, ElevationControls controls)
{
  return do_elevate(pointcloud, numPoints, ps, radius, controls);
}

bool elevate(const float* const pointcloud, std::size_t numPoints, const smtk::mesh::PointSet& ps,
  double radius, ElevationControls controls)
{
  return do_elevate(pointcloud, numPoints, ps, radius, controls);
}

bool elevate(const smtk::mesh::ElevationStructuredData& data, const smtk::mesh::PointSet& ps,
  double radius, ElevationControls controls)
{
  if (controls.m_clampMin && controls.m_clampMax)
  {
    ElevatePointForStructuredInput<true, true> functor(data, radius, controls);
    smtk::mesh::for_each(ps, functor);
  }
  else if (controls.m_clampMin)
  {
    ElevatePointForStructuredInput<true, false> functor(data, radius, controls);
    smtk::mesh::for_each(ps, functor);
  }
  else if (controls.m_clampMax)
  {
    ElevatePointForStructuredInput<false, true> functor(data, radius, controls);
    smtk::mesh::for_each(ps, functor);
  }
  else
  {
    ElevatePointForStructuredInput<false, false> functor(data, radius, controls);
    smtk::mesh::for_each(ps, functor);
  }

  return true;
}

bool elevate(const smtk::mesh::ElevationStructuredData& data, const smtk::mesh::MeshSet& ms,
  double radius, ElevationControls controls)
{
  return elevate(data, ms.points(), radius, controls);
}

bool elevate(const std::function<double(double, double)>& data, const smtk::mesh::PointSet& ps,
  ElevationControls controls)
{
  std::function<double(double, double)> clampedData;

  if (controls.m_clampMin && controls.m_clampMax)
  {
    clamper<true, true, double> clamp;
    clampedData = [=, &data](
      double x, double y) { return clamp(data(x, y), controls.m_minElev, controls.m_maxElev); };
  }
  else if (controls.m_clampMin)
  {
    clamper<true, false, double> clamp;
    clampedData = [=, &data](
      double x, double y) { return clamp(data(x, y), controls.m_minElev, controls.m_maxElev); };
  }
  else if (controls.m_clampMax)
  {
    clamper<false, true, double> clamp;
    clampedData = [=, &data](
      double x, double y) { return clamp(data(x, y), controls.m_minElev, controls.m_maxElev); };
  }
  else
  {
    clampedData = data;
  }
  ElevatePointForFunctionalInput functor(clampedData);
  smtk::mesh::for_each(ps, functor);

  return true;
}

bool elevate(const std::function<double(double, double)>& data, const smtk::mesh::MeshSet& ms,
  ElevationControls controls)
{
  return elevate(data, ms.points(), controls);
}

namespace
{
class DisplacePoint : public smtk::mesh::PointForEach
{
  smtk::mesh::PointLocator& m_locator;
  double m_radius;

public:
  DisplacePoint(smtk::mesh::PointLocator& locator, double radius)
    : m_locator(locator)
    , m_radius(radius)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    smtk::mesh::PointLocator::LocatorResults results;
    results.want_Coordinates = true;

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      m_locator.find(xyz[offset], xyz[offset + 1], xyz[offset + 2], m_radius, results);

      if (!results.pointIds.empty())
      {
        double sums[3] = { 0.0, 0.0, 0.0 };
        const std::size_t numPointsInRadius = results.x_s.size();
        for (std::size_t j = 0; j < numPointsInRadius; ++j)
        {
          sums[0] += results.x_s[j];
          sums[1] += results.y_s[j];
          sums[2] += results.z_s[j];
        }
        xyz[offset] = (sums[0] / numPointsInRadius);
        xyz[offset + 1] = (sums[1] / numPointsInRadius);
        xyz[offset + 2] = (sums[2] / numPointsInRadius);
      }
    }
    coordinatesModified = true; //mark we are going to modify the points
  }
};
}

bool displace(const smtk::mesh::PointSet& pointcloud, const smtk::mesh::MeshSet& ms, double radius)
{
  smtk::mesh::PointLocator locator(pointcloud);

  DisplacePoint functor(locator, radius);
  smtk::mesh::for_each(ms.points(), functor);
  return true;
}

bool displace(const smtk::mesh::PointSet& pointcloud, const smtk::mesh::PointSet& ps, double radius)
{
  smtk::mesh::PointLocator locator(pointcloud);

  DisplacePoint functor(locator, radius);
  smtk::mesh::for_each(ps, functor);
  return true;
}
}
}
