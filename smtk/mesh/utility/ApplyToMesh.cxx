//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/utility/ApplyToMesh.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/FieldTypes.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/PointSet.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

namespace smtk
{
namespace mesh
{
namespace utility
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
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    std::size_t offset = 0;
    std::array<double, 3> x, f_x;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i, offset += 3)
    {
      std::copy(xyz.data() + offset, xyz.data() + offset + 3, x.data());
      f_x = m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), xyz.data() + offset);
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
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping,
    std::size_t nPoints)
    : m_mapping(mapping)
    , m_data(3 * nPoints)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    std::size_t offset = 0;
    std::array<double, 3> x, f_x;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i, offset += 3)
    {
      std::copy(xyz.data() + offset, xyz.data() + offset + 3, x.data());

      std::copy(std::begin(x), std::end(x), m_data.data() + offset);

      f_x = m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), xyz.data() + offset);
    }
    coordinatesModified = true; //mark we are going to modify the points
  }

  [[nodiscard]] const std::vector<double>& data() const { return m_data; }
};

class UndoWarpPoints : public smtk::mesh::PointForEach
{
  std::vector<double> m_data;

public:
  UndoWarpPoints() = default;

  void forPoints(
    const smtk::mesh::HandleRange& /*pointIds*/,
    std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    xyz = m_data;
    coordinatesModified = true;
  }

  std::vector<double>& data() { return m_data; }
};
} // namespace

bool applyWarp(
  const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  smtk::mesh::MeshSet& ms,
  bool storePriorCoordinates)
{
  if (storePriorCoordinates)
  {
    StoreAndWarpPoints warp(f, ms.points().size());
    smtk::mesh::for_each(ms.points(), warp);
    return ms.createPointField("_prior", 3, smtk::mesh::FieldType::Double, warp.data().data())
      .isValid();
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
  pointfield.get(undoWarp.data().data());
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
  std::size_t m_counter{ 0 };

public:
  ScalarPointField(const std::function<double(std::array<double, 3>)>& mapping, std::size_t nPoints)
    : m_mapping(mapping)
    , m_data(nPoints)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    // The internal <m_counter> provides access to the the point field in
    // sequence. The local <xyzCounter> provides access to the coordinates of
    // the points currently being iterated. The iterator <i> provides access to
    // the memory space of the points (we currently use it for iteration).
    std::size_t xyzCounter = 0;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i, xyzCounter += 3)
    {
      m_data[m_counter++] = m_mapping(
        std::array<double, 3>({ { xyz[xyzCounter], xyz[xyzCounter + 1], xyz[xyzCounter + 2] } }));
    }
  }

  [[nodiscard]] const std::vector<double>& data() const { return m_data; }
};
} // namespace

bool applyScalarPointField(
  const std::function<double(std::array<double, 3>)>& f,
  const std::string& name,
  smtk::mesh::MeshSet& ms)
{
  ScalarPointField scalarPointField(f, ms.points().size());
  smtk::mesh::for_each(ms.points(), scalarPointField);
  return ms.createPointField(name, 1, smtk::mesh::FieldType::Double, scalarPointField.data().data())
    .isValid();
}

namespace
{
class ScalarCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<double(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter{ 0 };

public:
  ScalarCellField(const std::function<double(std::array<double, 3>)>& mapping, std::size_t nCells)
    : smtk::mesh::CellForEach(true)
    , m_mapping(mapping)
    , m_data(nCells)
  {
  }

  void forCell(const smtk::mesh::Handle& /*cellId*/, smtk::mesh::CellType /*cellType*/, int nPts)
    override
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
    m_data[m_counter++] = m_mapping(std::array<double, 3>({ { xyz[0], xyz[1], xyz[2] } }));
  }

  [[nodiscard]] const std::vector<double>& data() const { return m_data; }
};
} // namespace

bool applyScalarCellField(
  const std::function<double(std::array<double, 3>)>& f,
  const std::string& name,
  smtk::mesh::MeshSet& ms)
{
  ScalarCellField scalarCellField(f, ms.cells().size());
  smtk::mesh::for_each(ms.cells(), scalarCellField);
  return ms.createCellField(name, 1, smtk::mesh::FieldType::Double, scalarCellField.data().data())
    .isValid();
}

namespace
{
class VectorPointField : public smtk::mesh::PointForEach
{
private:
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter{ 0 };

public:
  VectorPointField(
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping,
    std::size_t nPoints)
    : m_mapping(mapping)
    , m_data(3 * nPoints)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    // The internal <m_counter> provides access to the the point field in
    // sequence. The local <xyzCounter> provides access to the coordinates of
    // the points currently being iterated. The iterator <i> provides access to
    // the memory space of the points (we currently use it for iteration).
    std::size_t xyzCounter = 0;
    std::array<double, 3> x, f_x;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i, xyzCounter += 3)
    {
      std::copy(xyz.data() + xyzCounter, xyz.data() + xyzCounter + 3, x.data());
      f_x = m_mapping(x);
      std::copy(std::begin(f_x), std::end(f_x), m_data.data() + m_counter);
      m_counter += 3;
    }
  }

  [[nodiscard]] const std::vector<double>& data() const { return m_data; }
};
} // namespace

bool applyVectorPointField(
  const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  const std::string& name,
  smtk::mesh::MeshSet& ms)
{
  VectorPointField vectorPointField(f, ms.points().size());
  smtk::mesh::for_each(ms.points(), vectorPointField);
  return ms.createPointField(name, 3, smtk::mesh::FieldType::Double, vectorPointField.data().data())
    .isValid();
}

namespace
{
class VectorCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<std::array<double, 3>(std::array<double, 3>)>& m_mapping;
  std::vector<double> m_data;
  std::size_t m_counter{ 0 };

public:
  VectorCellField(
    const std::function<std::array<double, 3>(std::array<double, 3>)>& mapping,
    std::size_t nCells)
    : smtk::mesh::CellForEach(true)
    , m_mapping(mapping)
    , m_data(3 * nCells)
  {
  }

  void forCell(const smtk::mesh::Handle& /*cellId*/, smtk::mesh::CellType /*cellType*/, int nPts)
    override
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
    f_x = m_mapping(x);
    std::copy(std::begin(f_x), std::end(f_x), m_data.data() + m_counter);
    m_counter += 3;
  }

  [[nodiscard]] const std::vector<double>& data() const { return m_data; }
};
} // namespace

bool applyVectorCellField(
  const std::function<std::array<double, 3>(std::array<double, 3>)>& f,
  const std::string& name,
  smtk::mesh::MeshSet& ms)
{
  VectorCellField vectorCellField(f, ms.cells().size());
  smtk::mesh::for_each(ms.cells(), vectorCellField);
  return ms.createCellField(name, 3, smtk::mesh::FieldType::Double, vectorCellField.data().data())
    .isValid();
}
} // namespace utility
} // namespace mesh
} // namespace smtk
