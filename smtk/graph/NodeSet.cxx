//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/graph/NodeSet.h"

#include "smtk/graph/Component.h"

namespace smtk
{
namespace graph
{

bool NodeSet::Compare::operator()(
  const std::shared_ptr<smtk::resource::Component>& lhs,
  const std::shared_ptr<smtk::resource::Component>& rhs) const
{
  return (!lhs ? true : (!rhs ? false : lhs->id() < rhs->id()));
}

const NodeSet::Container& NodeSet::nodes() const
{
  return m_nodes;
}

void NodeSet::visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const
{
  for (const auto& node : m_nodes)
  {
    v(node);
  }
}

NodeSet::NodeType NodeSet::find(const smtk::common::UUID& uuid) const
{
  struct QueryKey : resource::Component
  {
    QueryKey(const smtk::common::UUID& id)
      : m_id(id)
    {
    }
    const smtk::common::UUID& id() const final { return m_id; }
    bool setId(const smtk::common::UUID&) final { return false; }
    const smtk::resource::ResourcePtr resource() const final
    {
      return smtk::resource::ResourcePtr();
    }

    const smtk::common::UUID& m_id;
  };

  auto it = m_nodes.find(std::make_shared<QueryKey>(uuid));
  if (it != m_nodes.end())
  {
    return *it;
  }
  return std::shared_ptr<smtk::resource::Component>();
}

smtk::resource::Component* NodeSet::component(const smtk::common::UUID& uuid) const
{
  struct QueryKey : resource::Component
  {
    QueryKey(const smtk::common::UUID& id)
      : m_id(id)
    {
    }
    const smtk::common::UUID& id() const final { return m_id; }
    bool setId(const smtk::common::UUID&) final { return false; }
    const smtk::resource::ResourcePtr resource() const final
    {
      return smtk::resource::ResourcePtr();
    }

    const smtk::common::UUID& m_id;
  };

  auto it = m_nodes.find(std::make_shared<QueryKey>(uuid));
  if (it != m_nodes.end())
  {
    return it->get();
  }
  return nullptr;
}

std::size_t NodeSet::eraseNodes(const smtk::graph::ComponentPtr& node)
{
  return m_nodes.erase(node);
}

bool NodeSet::insertNode(const smtk::graph::ComponentPtr& node)
{
  return m_nodes.insert(node).second;
}

} // namespace graph
} // namespace smtk
