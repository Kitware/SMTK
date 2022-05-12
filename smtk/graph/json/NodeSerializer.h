//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_json_NodeSerializer_h
#define smtk_graph_json_NodeSerializer_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace graph
{

template<typename Resource>
struct NodeSerializer
{
  NodeSerializer(const Resource* resource, nlohmann::json& destination)
    : m_resource(resource)
    , m_destination(destination)
  {
  }

  template<typename NodeType>
  void evaluate(std::size_t ii) const
  {
    (void)ii;
    if (!m_resource)
    {
      return;
    }
    nlohmann::json jNodesOfType;
    std::string nodeType = smtk::common::typeName<NodeType>();
    // TODO: This is less than efficient as querying by nodeType (at least when
    //       using the default NodeSet container) visits all components.
    //       We perform this query once for each node type, which will be slow
    //       for large number of components.
    //       Furthermore, this query includes nodes that are subclasses of
    //       the requested class, which must then be rejected below.
    auto nodes = m_resource->template filterAs<std::unordered_set<std::shared_ptr<NodeType>>>(
      "'" + nodeType + "'");
    for (const auto& node : nodes)
    {
      if (node->typeName() != nodeType)
      {
        // Ignore subclasses.
        continue;
      }
      nlohmann::json jNode;
      to_json(jNode, node.get());
      jNodesOfType.push_back(jNode);
    }
    if (!jNodesOfType.empty())
    {
      m_destination[nodeType] = jNodesOfType;
    }
  }

  const Resource* m_resource = nullptr;
  nlohmann::json& m_destination;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_json_NodeSerializer_h
