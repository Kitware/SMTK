//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/core/Interface.h"

#include <cassert>

namespace smtk
{
namespace mesh
{

PointField::PointField()
  : m_name()
  , m_meshset()
{
}

PointField::PointField(const smtk::mesh::MeshSet& meshset, const std::string& name)
  : m_name(name)
  , m_meshset(meshset)
{
}

PointField::PointField(const smtk::mesh::PointField& other)

  = default;

PointField::~PointField() = default;

PointField& PointField::operator=(const PointField& other) = default;

bool PointField::operator==(const PointField& other) const
{
  return m_name == other.m_name && m_meshset == other.m_meshset;
}

bool PointField::operator!=(const PointField& other) const
{
  return !(*this == other);
}

bool PointField::operator<(const PointField& other) const
{
  if (m_name == other.m_name)
  {
    return m_meshset < other.m_meshset;
  }
  return m_name < other.m_name;
}

bool PointField::isValid() const
{
  if (m_name.empty() || m_meshset.resource() == nullptr)
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return false;
  }

  smtk::mesh::PointFieldTag dsTag(m_name);
  return iface->hasPointField(m_meshset.range(), dsTag);
}

std::size_t PointField::size() const
{
  return (this->isValid() ? m_meshset.points().size() : 0);
}

std::size_t PointField::dimension() const
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return 0;
  }

  smtk::mesh::PointFieldTag dsTag(m_name);
  return (
    iface->hasPointField(m_meshset.range(), dsTag) ? iface->getPointFieldDimension(dsTag) : 0);
}

smtk::mesh::FieldType PointField::type() const
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }

  smtk::mesh::PointFieldTag dsTag(m_name);
  return (iface->hasPointField(m_meshset.range(), dsTag) ? iface->getPointFieldType(dsTag)
                                                         : smtk::mesh::FieldType::MaxFieldType);
}

smtk::mesh::PointSet PointField::points() const
{
  return m_meshset.points();
}

bool PointField::get(const smtk::mesh::HandleRange& pointIds, void* values) const
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return false;
  }

  if (!smtk::mesh::rangeContains(m_meshset.points().range(), pointIds))
  {
    return false;
  }

  return iface->getField(pointIds, smtk::mesh::PointFieldTag(m_name), values);
}

bool PointField::set(const smtk::mesh::HandleRange& pointIds, const void* const values)
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return false;
  }

  if (!smtk::mesh::rangeContains(m_meshset.points().range(), pointIds))
  {
    return false;
  }

  return iface->setField(pointIds, smtk::mesh::PointFieldTag(m_name), values);
}

bool PointField::get(void* values) const
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->getPointField(m_meshset.range(), smtk::mesh::PointFieldTag(m_name), values);
}

bool PointField::set(const void* const values)
{
  const smtk::mesh::InterfacePtr& iface = m_meshset.resource()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->setPointField(m_meshset.range(), smtk::mesh::PointFieldTag(m_name), values);
}
}
}
