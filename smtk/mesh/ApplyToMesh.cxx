//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ApplyToMesh.h"

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/PointField.h"
#include "smtk/mesh/PointSet.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

namespace smtk
{
namespace mesh
{

namespace
{
class WarpPoints : public smtk::mesh::PointForEach
{
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;

public:
  WarpPoints(const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping)
    : m_mapping(mapping)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    std::array<double, 3> x, f_x;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      std::copy(&xyz[offset], &xyz[offset] + 3, &x[0]);
      f_x = this->m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), &xyz[offset]);
    }
    coordinatesModified = true; //mark we are going to modify the points
  }
};

class StoreAndWarpPoints : public smtk::mesh::PointForEach
{
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;

public:
  StoreAndWarpPoints(
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping, std::size_t nPoints)
    : m_mapping(mapping)
    , m_data(3 * nPoints)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool& coordinatesModified)
  {
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    std::array<double, 3> x, f_x;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, offset += 3)
    {
      std::copy(&xyz[offset], &xyz[offset] + 3, &x[0]);

      std::copy(std::begin(x), std::end(x), &this->m_data[offset]);

      f_x = this->m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), &xyz[offset]);
    }
    coordinatesModified = true; //mark we are going to modify the points
  }

  const std::vector<double>& data() const { return this->m_data; }
};

class UndoWarpPoints : public smtk::mesh::PointForEach
{
  std::vector<double> m_data;

public:
  UndoWarpPoints() {}

  void forPoints(
    const smtk::mesh::HandleRange&, std::vector<double>& xyz, bool& coordinatesModified)
  {
    xyz = this->m_data;
    coordinatesModified = true;
  }

  std::vector<double>& data() { return this->m_data; }
};
}

bool applyWarp(const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  smtk::mesh::MeshSet& ms, bool storePriorCoordinates)
{
  if (storePriorCoordinates)
  {
    StoreAndWarpPoints warp(f, ms.points().size());
    smtk::mesh::for_each(ms.points(), warp);
    return ms.createPointField("_prior", 3, &warp.data()[0]).isValid();
  }
  else
  {
    WarpPoints warp(f);
    smtk::mesh::for_each(ms.points(), warp);
    return true;
  }
}

bool undoWarp(smtk::mesh::MeshSet& ms)
{
  smtk::mesh::PointField pointfield = ms.pointField("_prior");
  if (!pointfield.isValid())
  {
    return false;
  }

  UndoWarpPoints undoWarp;
  undoWarp.data().resize(pointfield.size() * pointfield.dimension());
  pointfield.get(&undoWarp.data()[0]);
  smtk::mesh::for_each(ms.points(), undoWarp);
  return ms.removePointField(pointfield);
}

namespace
{
class ScalarPointField : public smtk::mesh::PointForEach
{
private:
  const std::function<double(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  ScalarPointField(const std::function<double(std::array<double, 3>)>& mapping, std::size_t nPoints)
    : smtk::mesh::PointForEach()
    , m_mapping(mapping)
    , m_data(nPoints)
    , m_counter(0)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&)
  {
    // The internal <m_counter> provides access to the the point field in
    // sequence. The local <xyzCounter> provides access to the coordinates of
    // the points currently being iterated. The iterator <i> provides access to
    // the memory space of the points (we currently use it for iteration).
    std::size_t xyzCounter = 0;
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, xyzCounter += 3)
    {
      this->m_data[this->m_counter++] = this->m_mapping(
        std::array<double, 3>({ { xyz[xyzCounter], xyz[xyzCounter + 1], xyz[xyzCounter + 2] } }));
    }
  }

  const std::vector<double>& data() const { return this->m_data; }
};
}

bool applyScalarPointField(const std::function<double(std::array<double, 3>)>& f,
  const std::string& name, smtk::mesh::MeshSet& ms)
{
  ScalarPointField scalarPointField(f, ms.points().size());
  smtk::mesh::for_each(ms.points(), scalarPointField);
  return ms.createPointField(name, 1, &scalarPointField.data()[0]).isValid();
}

namespace
{
class ScalarCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<double(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  ScalarCellField(const std::function<double(std::array<double, 3>)>& mapping, std::size_t nCells)
    : smtk::mesh::CellForEach(true)
    , m_mapping(mapping)
    , m_data(nCells)
    , m_counter(0)
  {
  }

  void forCell(const smtk::mesh::Handle&, smtk::mesh::CellType, int nPts)
  {
    double xyz[3] = { 0., 0., 0. };
    for (int i = 0; i < 3 * nPts; i += 3)
    {
      xyz[0] += this->coordinates()[i];
      xyz[1] += this->coordinates()[i + 1];
      xyz[2] += this->coordinates()[i + 2];
    }
    for (int i = 0; i < 3; i++)
    {
      xyz[i] /= nPts;
    }
    this->m_data[this->m_counter++] =
      this->m_mapping(std::array<double, 3>({ { xyz[0], xyz[1], xyz[2] } }));
  }

  const std::vector<double>& data() const { return this->m_data; }
};
}

bool applyScalarCellField(const std::function<double(std::array<double, 3>)>& f,
  const std::string& name, smtk::mesh::MeshSet& ms)
{
  ScalarCellField scalarCellField(f, ms.cells().size());
  smtk::mesh::for_each(ms.cells(), scalarCellField);
  return ms.createCellField(name, 1, &scalarCellField.data()[0]).isValid();
}

namespace
{
class VectorPointField : public smtk::mesh::PointForEach
{
private:
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  VectorPointField(
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping, std::size_t nPoints)
    : smtk::mesh::PointForEach()
    , m_mapping(mapping)
    , m_data(3 * nPoints)
    , m_counter(0)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&)
  {
    // The internal <m_counter> provides access to the the point field in
    // sequence. The local <xyzCounter> provides access to the coordinates of
    // the points currently being iterated. The iterator <i> provides access to
    // the memory space of the points (we currently use it for iteration).
    std::size_t xyzCounter = 0;
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::array<double, 3> x, f_x;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, xyzCounter += 3)
    {
      std::copy(&xyz[xyzCounter], &xyz[xyzCounter] + 3, &x[0]);
      f_x = this->m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), &this->m_data[this->m_counter]);
      this->m_counter += 3;
    }
  }

  const std::vector<double>& data() const { return this->m_data; }
};
}

bool applyVectorPointField(const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  const std::string& name, smtk::mesh::MeshSet& ms)
{
  VectorPointField vectorPointField(f, ms.points().size());
  smtk::mesh::for_each(ms.points(), vectorPointField);
  return ms.createPointField(name, 3, &vectorPointField.data()[0]).isValid();
}

namespace
{
class VectorCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  VectorCellField(
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping, std::size_t nCells)
    : smtk::mesh::CellForEach(true)
    , m_mapping(mapping)
    , m_data(3 * nCells)
    , m_counter(0)
  {
  }

  void forCell(const smtk::mesh::Handle&, smtk::mesh::CellType, int nPts)
  {
    std::array<double, 3> x = { { 0., 0., 0. } }, f_x;
    for (int i = 0; i < 3 * nPts; i += 3)
    {
      x[0] += this->coordinates()[i];
      x[1] += this->coordinates()[i + 1];
      x[2] += this->coordinates()[i + 2];
    }
    for (int i = 0; i < 3; i++)
    {
      x[i] /= nPts;
    }
    f_x = this->m_mapping(x);
    std::copy(std::begin(f_x), std::end(f_x), &this->m_data[this->m_counter]);
    this->m_counter += 3;
  }

  const std::vector<double>& data() const { return this->m_data; }
};
}

bool applyVectorCellField(const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  const std::string& name, smtk::mesh::MeshSet& ms)
{
  VectorCellField vectorCellField(f, ms.cells().size());
  smtk::mesh::for_each(ms.cells(), vectorCellField);
  return ms.createCellField(name, 3, &vectorCellField.data()[0]).isValid();
}
}
}
