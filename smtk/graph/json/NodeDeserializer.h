//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_json_NodeDeserializer_h
#define smtk_graph_json_NodeDeserializer_h

#include "smtk/graph/NodeProperties.h"

// #include "smtk/resource/json/Helper.h"

#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace graph
{

template<typename Resource>
struct NodeDeserializer
{
  NodeDeserializer(Resource* resource, const json& source)
    : m_resource(resource)
    , m_source(source)
  {
    if (!m_resource)
    {
      throw std::invalid_argument("Deserializer requires a non-null resource");
    }
  }

  template<typename NodeType>
  void evaluate(std::size_t ii) const
  {
    (void)ii;
    if (!m_resource)
    {
      return;
    }
    if (!NodeProperties<NodeType>::isSerializable::value)
    {
      return;
    }
    std::string nodeType = smtk::common::typeName<NodeType>();
    auto it = m_source.find(nodeType);
    if (it == m_source.end())
    {
      return;
    }
    // auto helper = smtk::resource::json::Helper::instance();
    const json& jNodesOfType(*it);
    for (const auto& jNode : jNodesOfType)
    {
      auto node = jNode.get<std::shared_ptr<NodeType>>();
      if (node)
      {
        m_resource->add(node);
      }
      // or alternately, m_resource->template create<NodeType>(jNode, helper);
    }
  }

  Resource* m_resource = nullptr;
  const json& m_source;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_json_NodeDeserializer_h
