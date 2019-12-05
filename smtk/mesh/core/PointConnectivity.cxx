//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/PointConnectivity.h"
#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace mesh
{

PointConnectivity::PointConnectivity(
  const smtk::mesh::ResourcePtr& parent, const smtk::mesh::HandleRange& range)
  : m_parent(parent)
  , m_connectivity(parent->interface()->connectivityStorage(range))
  , m_iteratorLocation()
{
}

PointConnectivity::PointConnectivity(const smtk::mesh::PointConnectivity& other)
  : m_parent(other.m_parent)
  , m_connectivity(other.m_connectivity)
{
}

PointConnectivity::~PointConnectivity() = default;

PointConnectivity& PointConnectivity::operator=(const PointConnectivity& other)
{
  m_parent = other.m_parent;
  m_connectivity = other.m_connectivity;
  return *this;
}

bool PointConnectivity::operator==(const PointConnectivity& other) const
{
  return m_parent == other.m_parent && m_connectivity->equal(other.m_connectivity.get());
}

bool PointConnectivity::operator!=(const PointConnectivity& other) const
{
  return !(*this == other);
}

std::size_t PointConnectivity::size() const
{
  return m_connectivity->vertSize();
}

std::size_t PointConnectivity::numberOfCells() const
{
  return m_connectivity->cellSize();
}

bool PointConnectivity::is_empty() const
{
  return m_connectivity->cellSize() == 0;
}

void PointConnectivity::initCellTraversal()
{
  //we store the iteration of the traversal inside ourselves not inside
  //the connectivity. The primary reason for this is that the connectivity
  //can be shared between multiple instance of connectivity, but each one
  //should be able to iterate over the shared data

  m_connectivity->initTraversal(m_iteratorLocation);
}

bool PointConnectivity::fetchNextCell(int& numPts, const smtk::mesh::Handle*& points)
{
  smtk::mesh::CellType cellType;
  return m_connectivity->fetchNextCell(m_iteratorLocation, cellType, numPts, points);
}

bool PointConnectivity::fetchNextCell(
  smtk::mesh::CellType& cellType, int& numPts, const smtk::mesh::Handle*& points)
{
  return m_connectivity->fetchNextCell(m_iteratorLocation, cellType, numPts, points);
}
}
}
