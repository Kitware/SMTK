//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/NodeSet.h"

#include "smtk/markup/AssignedIds.h"

namespace smtk
{
namespace markup
{

NodeSet::~NodeSet() = default;

void NodeSet::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;
  (void)data;
}

bool NodeSet::setDomain(const std::weak_ptr<smtk::markup::AssignedIds>& domain)
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

const std::weak_ptr<smtk::markup::AssignedIds>& NodeSet::domain() const
{
  return m_domain;
}

std::weak_ptr<smtk::markup::AssignedIds>& NodeSet::domain()
{
  return m_domain;
}

bool NodeSet::setNodes(
  const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& nodes)
{
  if (m_nodes == nodes)
  {
    return false;
  }
  m_nodes = nodes;
  return true;
}

const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
NodeSet::nodes() const
{
  return m_nodes;
}

std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
NodeSet::nodes()
{
  return m_nodes;
}

} // namespace markup
} // namespace smtk
