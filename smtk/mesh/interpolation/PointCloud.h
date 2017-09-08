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

namespace smtk
{
namespace mesh
{

/**\brief A wrapper for point cloud data.

   This class is a facade for describing external data sets that consist of
   unstructured points and associated scalar values. While there is convenience
   API for converting arrays of coordinates and data into this format, the
   general use of this class is to pass it a size <nPoints> and two functors
   <coordinates> and <data>. <coordinates> is an I->R^3 function for accessing
   the ith coordinate of the data set, and <data> is an I->R function for
   accessing the scalar value associated with the ith point.
  */
class SMTKCORE_EXPORT PointCloud
{
public:
  PointCloud(std::size_t nPoints,
    const std::function<std::array<double, 3>(std::size_t)>& coordinates,
    const std::function<double(std::size_t)>& data)
    : m_size(nPoints)
    , m_coordinates(coordinates)
    , m_data(data)
  {
  }

  PointCloud(std::size_t nPoints, const double* const coordinates, const double* const data)
    : PointCloud(nPoints,
        [=](std::size_t i) {
          return std::array<double, 3>(
            { { coordinates[3 * i], coordinates[3 * i + 1], coordinates[3 * i + 2] } });
        },
        [=](std::size_t i) { return data[i]; })
  {
  }

  PointCloud(std::size_t nPoints, const float* const coordinates, const float* const data)
    : PointCloud(nPoints,
        [=](std::size_t i) {
          return std::array<double, 3>(
            { { coordinates[3 * i], coordinates[3 * i + 1], coordinates[3 * i + 2] } });
        },
        [=](std::size_t i) { return data[i]; })
  {
  }

  PointCloud(const std::vector<double>& coordinates, const std::vector<double>& data)
    : PointCloud(data.size(), coordinates.data(), data.data())
  {
    assert(coordinates.size() == 3 * data.size());
  }

  virtual ~PointCloud() {}

  std::size_t size() const { return m_size; }

  const std::function<std::array<double, 3>(std::size_t)>& coordinates() const
  {
    return m_coordinates;
  }
  const std::function<double(std::size_t)>& data() const { return m_data; }

protected:
  std::size_t m_size;
  const std::function<std::array<double, 3>(std::size_t)> m_coordinates;
  const std::function<double(std::size_t)> m_data;
};
}
}

#endif
