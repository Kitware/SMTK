//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_json_NodeInitializer_h
#define smtk_markup_json_NodeInitializer_h

#include "smtk/graph/NodeProperties.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace markup
{

/**\brief Initialize nodes after they have been deserialized.
  *
  * At the point this functor's evaluate() method is invoked,
  * all nodes and arcs in the resource have been created but
  * the nodes have not been initialized.
  * This functor gives nodes a chance to update references to
  * other nodes by following arcs without having to worry
  * about deserialization order. (Note that you cannot depend
  * on the order of nodes being initialized, only that they
  * all exist at this point and have all their arcs in place.)
  */
template<typename Resource>
struct NodeInitializer
{
  NodeInitializer(Resource* resource, const json& source)
    : m_resource(resource)
    , m_source(source)
  {
    if (!m_resource)
    {
      throw std::invalid_argument("Initializer requires a non-null resource");
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
    if (!graph::NodeProperties<NodeType>::isSerializable::value)
    {
      return;
    }
    std::string nodeType = smtk::common::typeName<NodeType>();
    auto it = m_source.find(nodeType);
    if (it == m_source.end())
    {
      return;
    }
    auto& helper = smtk::resource::json::Helper::instance();
    const json& jNodesOfType(*it);
    for (const auto& jNode : jNodesOfType)
    {
      auto idit = jNode.find("id");
      if (idit == jNode.end())
      {
        continue;
      }
      auto* comp = dynamic_cast<smtk::markup::Component*>(
        m_resource->component(idit->get<smtk::common::UUID>()));
      if (comp)
      {
        comp->initialize(jNode, helper);
      }
    }
  }

  Resource* m_resource = nullptr;
  const json& m_source;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_json_NodeInitializer_h
