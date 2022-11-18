//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/SideSet.h"

#include "smtk/markup/AssignedIds.h"

namespace smtk
{
namespace markup
{

SideSet::~SideSet() = default;

void SideSet::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;
  (void)data;
}

bool SideSet::setDomain(const std::weak_ptr<smtk::markup::AssignedIds>& domain)
{
  auto mlocked = m_domain.lock();
  auto vlocked = domain.lock();
  if (mlocked == vlocked)
  {
    return false;
  }
  m_domain = vlocked;
  return true;
}

const std::weak_ptr<smtk::markup::AssignedIds>& SideSet::domain() const
{
  return m_domain;
}

std::weak_ptr<smtk::markup::AssignedIds>& SideSet::domain()
{
  return m_domain;
}

bool SideSet::setBoundaryOperator(
  const std::weak_ptr<smtk::markup::BoundaryOperator>& boundaryOperator)
{
  auto mlocked = m_boundaryOperator.lock();
  auto vlocked = boundaryOperator.lock();
  if (mlocked == vlocked)
  {
    return false;
  }
  m_boundaryOperator = vlocked;
  return true;
}

const std::weak_ptr<smtk::markup::BoundaryOperator>& SideSet::boundaryOperator() const
{
  return m_boundaryOperator;
}

std::weak_ptr<smtk::markup::BoundaryOperator>& SideSet::boundaryOperator()
{
  return m_boundaryOperator;
}

bool SideSet::setSides(
  const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& sides)
{
  if (m_sides == sides)
  {
    return false;
  }
  m_sides = sides;
  return true;
}

const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
SideSet::sides() const
{
  return m_sides;
}

std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
SideSet::sides()
{
  return m_sides;
}

} // namespace markup
} // namespace smtk
