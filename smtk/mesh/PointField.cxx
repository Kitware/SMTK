//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/PointField.h"
#include "smtk/mesh/Collection.h"

#include "smtk/mesh/Interface.h"

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
  : m_name(other.m_name)
  , m_meshset(other.m_meshset)
{
}

PointField::~PointField()
{
}

PointField& PointField::operator=(const PointField& other)
{
  this->m_name = other.m_name;
  this->m_meshset = other.m_meshset;
  return *this;
}

bool PointField::operator==(const PointField& other) const
{
  return this->m_name == other.m_name && this->m_meshset == other.m_meshset;
}

bool PointField::operator!=(const PointField& other) const
{
  return !(*this == other);
}

bool PointField::operator<(const PointField& other) const
{
  if (this->m_name == other.m_name)
  {
    return this->m_meshset < other.m_meshset;
  }
  return this->m_name < other.m_name;
}

bool PointField::isValid() const
{
  if (this->m_name.empty() || this->m_meshset.collection() == nullptr)
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  smtk::mesh::PointFieldTag dsTag(this->m_name);
  return iface->hasPointField(this->m_meshset.range(), dsTag);
}

std::size_t PointField::size() const
{
  return (this->isValid() ? this->m_meshset.points().size() : 0);
}

std::size_t PointField::dimension() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return 0;
  }

  smtk::mesh::PointFieldTag dsTag(this->m_name);
  return (iface->hasPointField(this->m_meshset.range(), dsTag)
      ? iface->getPointFieldDimension(dsTag)
      : 0);
}

smtk::mesh::PointSet PointField::points() const
{
  return this->m_meshset.points();
}

std::vector<double> PointField::get(const smtk::mesh::HandleRange& pointIds) const
{
  std::vector<double> values;
  values.resize(pointIds.size() * this->dimension());
  this->get(pointIds, &values[0]);
  return values;
}

bool PointField::set(const smtk::mesh::HandleRange& pointIds, const std::vector<double>& values)
{
  assert(values.size() >= pointIds.size() * this->dimension());
  return this->set(pointIds, &values[0]);
}

std::vector<double> PointField::get() const
{
  std::vector<double> values;
  values.resize(this->size() * this->dimension());
  this->get(&values[0]);
  return values;
}

bool PointField::set(const std::vector<double>& values)
{
  assert(values.size() >= this->size() * this->dimension());
  return this->set(&values[0]);
}

bool PointField::get(const smtk::mesh::HandleRange& pointIds, double* values) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  if (!this->m_meshset.points().range().contains(pointIds))
  {
    return false;
  }

  return iface->getField(pointIds, smtk::mesh::PointFieldTag(this->m_name), values);
}

bool PointField::set(const smtk::mesh::HandleRange& pointIds, const double* const values)
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  if (!this->m_meshset.points().range().contains(pointIds))
  {
    return false;
  }

  return iface->setField(pointIds, smtk::mesh::PointFieldTag(this->m_name), values);
}

bool PointField::get(double* values) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->getPointField(
    this->m_meshset.range(), smtk::mesh::PointFieldTag(this->m_name), values);
}

bool PointField::set(const double* const values)
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->setPointField(
    this->m_meshset.range(), smtk::mesh::PointFieldTag(this->m_name), values);
}
}
}
