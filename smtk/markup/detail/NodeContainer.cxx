//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/detail/NodeContainer.h"

namespace smtk
{
namespace markup
{
namespace detail
{

void NodeContainer::visit(Visitor visitor) const
{
  const auto& nodesById = m_nodes.get<IdTag>();
  for (const auto& node : nodesById)
  {
    auto component = std::dynamic_pointer_cast<smtk::resource::Component>(node);
    visitor(component);
  }
}

smtk::resource::ComponentPtr NodeContainer::find(const smtk::common::UUID& uuid) const
{
  smtk::resource::ComponentPtr result;
  const auto& nodesById = m_nodes.get<IdTag>();
  auto it = nodesById.find(uuid);
  if (it != nodesById.end())
  {
    result = *it;
  }
  return result;
}

smtk::resource::Component* NodeContainer::component(const smtk::common::UUID& uuid) const
{
  smtk::resource::Component* result = nullptr;
  const auto& nodesById = m_nodes.get<IdTag>();
  auto it = nodesById.find(uuid);
  if (it != nodesById.end())
  {
    result = it->get();
  }
  return result;
}

std::size_t NodeContainer::eraseNodes(const smtk::resource::ComponentPtr& node)
{
  std::size_t didErase = 0;
  auto graphNode = std::dynamic_pointer_cast<Component>(node);
  if (!graphNode)
  {
    return didErase;
  }
  auto& nodesById = m_nodes.get<IdTag>();
  auto it = nodesById.find(graphNode->id());
  if (it != nodesById.end())
  {
    nodesById.erase(node->id());
    didErase = 1;
  }
  return didErase;
}

bool NodeContainer::insertNode(const smtk::resource::ComponentPtr& node)
{
  bool didInsert = false;
  auto graphNode = std::dynamic_pointer_cast<Component>(node);
  if (!graphNode)
  {
    return didInsert;
  }
  auto& nodesById = m_nodes.get<IdTag>();
  auto result = nodesById.insert(graphNode);
  return result.second;
}

} // namespace detail
} // namespace markup
} // namespace smtk
