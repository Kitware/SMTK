//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/PointSet.h"

#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace mesh
{

PointSet::PointSet(const smtk::mesh::ResourcePtr& parent, const smtk::mesh::HandleRange& points)
  : m_parent(parent)
  , m_points(points)
{
}

PointSet::PointSet(
  const smtk::mesh::ConstResourcePtr& parent,
  const smtk::mesh::HandleRange& points)
  : m_parent(std::const_pointer_cast<smtk::mesh::Resource>(parent))
  , m_points(points)
{
}

PointSet::PointSet(
  const smtk::mesh::ResourcePtr& parent,
  const std::vector<smtk::mesh::Handle>& points)
  : m_parent(parent)
{
  for (const auto& point : points)
  {
    m_points.insert(point);
  }
}

PointSet::PointSet(
  const smtk::mesh::ResourcePtr& parent,
  const std::set<smtk::mesh::Handle>& points)
  : m_parent(parent)
{
  for (const auto& point : points)
  {
    m_points.insert(point);
  }
}

PointSet::PointSet(const smtk::mesh::PointSet& other) = default;

PointSet::~PointSet() = default;

PointSet& PointSet::operator=(const PointSet& other)
{
  m_parent = other.m_parent;
  m_points = other.m_points;
  return *this;
}

bool PointSet::operator==(const PointSet& other) const
{
  return m_parent == other.m_parent && m_points == other.m_points;
}

bool PointSet::operator!=(const PointSet& other) const
{
  return !(*this == other);
}

bool PointSet::is_empty() const
{
  return m_points.empty();
}

std::size_t PointSet::size() const
{
  return m_points.size();
}

std::size_t PointSet::numberOfPoints() const
{
  return m_points.size();
}

bool PointSet::contains(const smtk::mesh::Handle& pointId) const
{
  return m_points.find(pointId) != m_points.end();
}

std::size_t PointSet::find(const smtk::mesh::Handle& pointId) const
{
  return smtk::mesh::rangeIndex(m_points, pointId);
}

bool PointSet::get(double* xyz) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->getCoordinates(m_points, xyz);
}

bool PointSet::get(std::vector<double>& xyz) const
{
  const std::size_t size = this->numberOfPoints();
  xyz.resize(size * 3);

  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->getCoordinates(m_points, xyz.data());
}

bool PointSet::get(float* xyz) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->getCoordinates(m_points, xyz);
}

bool PointSet::get(std::vector<float>& xyz) const
{
  const std::size_t size = this->numberOfPoints();
  xyz.resize(size * 3);

  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->getCoordinates(m_points, xyz.data());
}

bool PointSet::set(const double* const xyz) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setCoordinates(m_points, xyz);
}

bool PointSet::set(const std::vector<double>& xyz) const
{
  if (xyz.size() < this->numberOfPoints() * 3)
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setCoordinates(m_points, xyz.data());
}

bool PointSet::set(const float* const xyz)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setCoordinates(m_points, xyz);
}

bool PointSet::set(const std::vector<float>& xyz)
{
  if (xyz.size() < this->numberOfPoints() * 3)
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setCoordinates(m_points, xyz.data());
}

/**\brief Get the parent resource that this meshset belongs to.
  *
  */
const smtk::mesh::ResourcePtr& PointSet::resource() const
{
  return m_parent;
}

PointSet set_intersect(const PointSet& a, const PointSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty PointSet if the resources don't match
    return smtk::mesh::PointSet(a.m_parent, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_points & b.m_points;
  return smtk::mesh::PointSet(a.m_parent, result);
}

PointSet set_difference(const PointSet& a, const PointSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty PointSet if the resources don't match
    return smtk::mesh::PointSet(a.m_parent, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_points - b.m_points;
  return smtk::mesh::PointSet(a.m_parent, result);
}

PointSet set_union(const PointSet& a, const PointSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty PointSet if the resources don't match
    return smtk::mesh::PointSet(a.m_parent, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_points | b.m_points;
  return smtk::mesh::PointSet(a.m_parent, result);
}

void for_each(const PointSet& a, PointForEach& filter)
{
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  filter.m_resource = a.m_parent;
  iface->pointForEach(a.m_points, filter);
}
} // namespace mesh
} // namespace smtk
