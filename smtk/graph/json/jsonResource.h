//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_json_jsonResource_h
#define smtk_graph_json_jsonResource_h

#include "smtk/CoreExports.h"

#include "smtk/graph/NodeProperties.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/json/ArcDeserializer.h"
#include "smtk/graph/json/ArcSerializer.h"
#include "smtk/graph/json/NodeDeserializer.h"
#include "smtk/graph/json/NodeSerializer.h"

#include "smtk/resource/json/Helper.h"
#include "smtk/resource/json/jsonResource.h"

#include "smtk/common/json/jsonUUID.h"

#include "nlohmann/json.hpp"

// Define how graphs are serialized.
namespace smtk
{
namespace graph
{
using json = nlohmann::json;

template<typename NodeType, typename PreviousResult>
struct AnySerializableNodeTypes
{
  using type = typename std::conditional<
    NodeProperties<NodeType>::isSerializable::value,
    std::true_type,
    PreviousResult>::type;
};

template<typename Traits>
void to_json(json& j, const std::shared_ptr<Resource<Traits>>& resource)
{
  // First, serialize the resource ID, location, type, links, and properties:
  smtk::resource::to_json(j, resource);

  // If we have a name, save it:
  if (resource->isNameSet())
  {
    j["name"] = resource->name();
  }

  // Only include arcs if 1 or more arc containers serialize 1 or more arcs.
  json arcData;
  resource->template evaluateArcs<ArcSerializer>(resource, arcData);
  if (!arcData.empty())
  {
    j["arcs"] = arcData;
  }

  // Only include nodes if 1 or more node types is marked for storage
  // (otherwise, nodes are assumed to be implicit).
  typename smtk::
    tuple_reduce<AnySerializableNodeTypes, typename Traits::NodeTypes, std::false_type>::type
      haveNodes;
  if (haveNodes)
  {
    json nodeData;
    NodeSerializer<Resource<Traits>> nodeSerializer(resource.get(), nodeData);
    smtk::tuple_evaluate<typename Traits::NodeTypes>(nodeSerializer);
    if (!nodeData.empty())
    {
      j["nodes"] = nodeData;
    }
  }
}

template<typename Traits>
void from_json(const json& j, std::shared_ptr<Resource<Traits>>& resource)
{
  if (!resource)
  {
    auto& helper = smtk::resource::json::Helper::instance();
    resource = std::dynamic_pointer_cast<Resource<Traits>>(helper.resource());
    if (!resource)
    {
      resource = Resource<Traits>::create();
      smtk::resource::json::Helper::pushInstance(resource);
    }
  }
  smtk::resource::ResourcePtr parentResource(resource);
  smtk::resource::from_json(j, parentResource);

  if (j.find("name") != j.end())
  {
    resource->setName(j.at("name"));
  }

  // If no node types are marked for serialization, then
  // we do not need to deserialize nodes. (Even if nodes
  // are present in the JSON, they are not allowed to be
  // processed.)
  typename smtk::
    tuple_reduce<AnySerializableNodeTypes, typename Traits::NodeTypes, std::false_type>::type
      haveNodes;
  auto nodeIt = j.find("nodes");
  if (nodeIt != j.end())
  {
    if (!haveNodes)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "JSON data has explicit nodes, but no node type in the resource's "
        "traits are marked as serializable. These nodes will be ignored.");
    }
    else
    {
      NodeDeserializer<Resource<Traits>> deserializer(resource.get(), *nodeIt);
      smtk::tuple_evaluate<typename Traits::NodeTypes>(deserializer);
    }
  }

  // If any arcs have been serialized, deserialize them.
  if (j.find("arcs") != j.end())
  {
    resource->template evaluateArcs<ArcDeserializer>(resource, j.at("arcs"));
  }
}

} // namespace graph
} // namespace smtk

#endif
