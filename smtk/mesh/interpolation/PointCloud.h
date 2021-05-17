//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointCloud_h
#define __smtk_mesh_PointCloud_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <array>
#include <cassert>
#include <functional>
#include <limits>

namespace smtk
{
namespace mesh
{

/**\brief A wrapper for point cloud data.

   This class is a facade for describing external data sets that consist of
   unstructured points and associated scalar values. While there is convenience
   API for converting arrays of coordinates and data into this format, the
   general use of this class is to pass it a size \a nPoints and two functors
   \a coordinates and \a data. \a coordinates is an I->R^3 function for accessing
   the ith coordinate of the data set, and \a data is an I->R function for
   accessing the scalar value associated with the ith point.
  */
class SMTKCORE_EXPORT PointCloud
{
public:
  PointCloud(
    std::size_t nPoints,
    const std::function<std::array<double, 3>(std::size_t)>& coordinates,
    const std::function<double(std::size_t)>& data,
    const std::function<bool(std::size_t)>& valid)
    : m_size(nPoints)
    , m_coordinates(coordinates)
    , m_data(data)
    , m_valid(valid)
  {
  }

  /// Returns an invalid PointCloud.
  PointCloud()
    : PointCloud(
        0,
        [](std::size_t) {
          double nan = std::numeric_limits<double>::quiet_NaN();
          return std::array<double, 3>({ { nan, nan, nan } });
        },
        [](std::size_t) { return std::numeric_limits<double>::quiet_NaN(); },
        [](std::size_t) { return false; })
  {
  }

  /// Constructs a PointCloud with no blanking (all points are considered valid).
  PointCloud(
    std::size_t nPoints,
    const std::function<std::array<double, 3>(std::size_t)>& coordinates,
    const std::function<double(std::size_t)>& data)
    : m_size(nPoints)
    , m_coordinates(coordinates)
    , m_data(data)
    , m_valid([](std::size_t) { return true; })
  {
  }

  /// Constructs a PointCloud from arrays of coordinates and data. The arrays
  /// must remain in scope for the lifetime of the PointCloud.
  PointCloud(std::size_t nPoints, const double* const coordinates, const double* const data)
    : PointCloud(
        nPoints,
        [=](std::size_t i) {
          return std::array<double, 3>(
            { { coordinates[3 * i], coordinates[3 * i + 1], coordinates[3 * i + 2] } });
        },
        [=](std::size_t i) { return data[i]; },
        [](std::size_t) { return true; })
  {
  }

  /// Constructs a PointCloud from arrays of coordinates and data. The arrays
  /// must remain in scope for the lifetime of the PointCloud.
  PointCloud(std::size_t nPoints, const float* const coordinates, const float* const data)
    : PointCloud(
        nPoints,
        [=](std::size_t i) {
          return std::array<double, 3>(
            { { coordinates[3 * i], coordinates[3 * i + 1], coordinates[3 * i + 2] } });
        },
        [=](std::size_t i) { return data[i]; },
        [](std::size_t) { return true; })
  {
  }

private:
  struct Coordinates
  {
    Coordinates(std::vector<double> coords)
      : m_coordinates(std::move(coords))
    {
    }

    std::array<double, 3> operator()(std::size_t i) const
    {
      return std::array<double, 3>(
        { { m_coordinates[3 * i], m_coordinates[3 * i + 1], m_coordinates[3 * i + 2] } });
    }

    const std::vector<double> m_coordinates;
  };

  struct Data
  {
    Data(std::vector<double> data)
      : m_data(std::move(data))
    {
    }

    double operator()(std::size_t i) const { return m_data[i]; }

    const std::vector<double> m_data;
  };

public:
  /// Constructs a PointCloud from vectors of coordinates and data.
  PointCloud(std::vector<double>&& coordinates, std::vector<double>&& data)
    : PointCloud(data.size(), Coordinates(coordinates), Data(data), [](std::size_t) {
      return true;
    })
  {
  }

  virtual ~PointCloud() = default;

  std::size_t size() const { return m_size; }

  const std::function<std::array<double, 3>(std::size_t)>& coordinates() const
  {
    return m_coordinates;
  }
  const std::function<double(std::size_t)>& data() const { return m_data; }

  /// Given an index into the structured data, determine whether or not the point
  /// is valid.
  bool containsIndex(std::size_t i) const { return m_valid(i); }

protected:
  std::size_t m_size;
  std::function<std::array<double, 3>(std::size_t)> m_coordinates;
  std::function<double(std::size_t)> m_data;
  std::function<bool(std::size_t)> m_valid;
};
} // namespace mesh
} // namespace smtk

#endif
