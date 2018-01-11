//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Collection.h"

#include "smtk/mesh/core/Interface.h"

#include <cassert>

namespace smtk
{
namespace mesh
{

CellField::CellField()
  : m_name()
  , m_meshset()
{
}

CellField::CellField(const smtk::mesh::MeshSet& meshset, const std::string& name)
  : m_name(name)
  , m_meshset(meshset)
{
}

CellField::CellField(const smtk::mesh::CellField& other)
  : m_name(other.m_name)
  , m_meshset(other.m_meshset)
{
}

CellField::~CellField()
{
}

CellField& CellField::operator=(const CellField& other)
{
  this->m_name = other.m_name;
  this->m_meshset = other.m_meshset;
  return *this;
}

bool CellField::operator==(const CellField& other) const
{
  return this->m_name == other.m_name && this->m_meshset == other.m_meshset;
}

bool CellField::operator!=(const CellField& other) const
{
  return !(*this == other);
}

bool CellField::operator<(const CellField& other) const
{
  if (this->m_name == other.m_name)
  {
    return this->m_meshset < other.m_meshset;
  }
  return this->m_name < other.m_name;
}

bool CellField::isValid() const
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

  smtk::mesh::CellFieldTag dsTag(this->m_name);
  return iface->hasCellField(this->m_meshset.range(), dsTag);
}

std::size_t CellField::size() const
{
  return (this->isValid() ? this->m_meshset.cells().size() : 0);
}

std::size_t CellField::dimension() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return 0;
  }

  smtk::mesh::CellFieldTag dsTag(this->m_name);
  return (
    iface->hasCellField(this->m_meshset.range(), dsTag) ? iface->getCellFieldDimension(dsTag) : 0);
}

smtk::mesh::FieldType CellField::type() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }

  smtk::mesh::CellFieldTag dsTag(this->m_name);
  return (iface->hasCellField(this->m_meshset.range(), dsTag)
      ? iface->getCellFieldType(dsTag)
      : smtk::mesh::FieldType::MaxFieldType);
}

smtk::mesh::CellSet CellField::cells() const
{
  return this->m_meshset.cells();
}

bool CellField::get(const smtk::mesh::HandleRange& cellIds, void* values) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  if (!this->m_meshset.cells().range().contains(cellIds))
  {
    return false;
  }

  return iface->getField(cellIds, smtk::mesh::CellFieldTag(this->m_name), values);
}

bool CellField::set(const smtk::mesh::HandleRange& cellIds, const void* const values)
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  if (!this->m_meshset.cells().range().contains(cellIds))
  {
    return false;
  }

  return iface->setField(cellIds, smtk::mesh::CellFieldTag(this->m_name), values);
}

bool CellField::get(void* values) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->getCellField(
    this->m_meshset.range(), smtk::mesh::CellFieldTag(this->m_name), values);
}

bool CellField::set(const void* const values)
{
  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  if (!iface)
  {
    return false;
  }

  return iface->setCellField(
    this->m_meshset.range(), smtk::mesh::CellFieldTag(this->m_name), values);
}
}
}
