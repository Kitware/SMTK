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
#include "smtk/graph/operators/CreateArc.h"
#include "smtk/graph/operators/DeleteArc.h"

#include "smtk/operation/groups/ArcCreator.h"

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
  using namespace smtk::string::literals;

  // First, serialize the resource ID, location, type, links, and properties:
  smtk::resource::to_json(j, resource);

  // If we have a name, save it:
  if (resource->isNameSet())
  {
    j["name"] = resource->name();
  }

  // Include run-time arc definitions if any are present.
  json::array_t runtimeArcTypes;
  for (const auto& runtimeArcBases : resource->arcs().runtimeBaseTypes())
  {
    for (const auto& runtimeArcType : resource->arcs().runtimeTypeNames(runtimeArcBases))
    {
      const auto& arcType =
        resource->arcs().template getRuntime<ArcImplementationBase>(runtimeArcType);
      json arcJSON;
      arcJSON["arc-type"] = runtimeArcType.data();
      arcJSON["directionality"] = directionalityToken(arcType.directionality()).data();
      switch (arcType.directionality())
      {
        case Directionality::IsDirected:
          arcJSON["from-types"] = arcType.fromTypes();
          arcJSON["to-types"] = arcType.toTypes();
          break;
        case Directionality::IsUndirected:
          arcJSON["end-types"] = arcType.fromTypes();
          break;
      }
      runtimeArcTypes.push_back(arcJSON);
    }
  }
  if (!runtimeArcTypes.empty())
  {
    j["arc-types"] = runtimeArcTypes;
  }

  // Only include arcs if 1 or more arc containers serialize 1 or more arcs.
  json arcData;
  resource->template evaluateArcs<ArcSerializer>(arcData);
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
  auto& helper = smtk::resource::json::Helper::instance();
  if (!resource)
  {
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

  // Before we deserialize arcs, deserialize any run-time arc types.
  if (j.find("arc-types") != j.end())
  {
    for (const auto& arcEntry : j.at("arc-types"))
    {
      smtk::string::Token arcTypeName = arcEntry.at("arc-type").get<std::string>();
      auto dir =
        smtk::graph::directionalityEnumerant(arcEntry.at("directionality").get<std::string>());
      std::unordered_set<smtk::string::Token> fromTypes;
      std::unordered_set<smtk::string::Token> toTypes;
      switch (dir)
      {
        case Directionality::IsDirected:
          fromTypes = arcEntry.at("from-types").get<std::unordered_set<smtk::string::Token>>();
          toTypes = arcEntry.at("to-types").get<std::unordered_set<smtk::string::Token>>();
          break;
        case Directionality::IsUndirected:
          fromTypes = arcEntry.at("end-types").get<std::unordered_set<smtk::string::Token>>();
          toTypes = fromTypes;
          break;
      }
      bool didInsert =
        resource->arcs().insertRuntimeArcType(resource.get(), arcTypeName, fromTypes, toTypes, dir);
      if (!didInsert)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Failed to add run-time arc type \"" << arcTypeName.data() << "\".");
      }
      else
      {
        auto managers = helper.managers();
        auto opMgr = managers && managers->contains<smtk::operation::Manager::Ptr>()
          ? managers->get<smtk::operation::Manager::Ptr>()
          : smtk::operation::Manager::Ptr();
        if (opMgr)
        {
          smtk::operation::ArcCreator creatorGroup(opMgr);
          if (!creatorGroup.registerOperation<smtk::graph::CreateArc>(
                { { arcTypeName.data() } }, "to node"))
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Could not register arc creator for \"" << arcTypeName.data() << "\" arcs.");
          }
        }
        if (!DeleteArc::registerDeleter(
              arcTypeName, resource->typeToken(), fromTypes, toTypes, opMgr))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not register arc deleter for \"" << arcTypeName.data() << "\" arcs.");
        }
      }
    }
  }

  // If any arcs have been serialized, deserialize them.
  if (j.find("arcs") != j.end())
  {
    resource->template evaluateArcs<ArcDeserializer>(j.at("arcs"));
  }
}

} // namespace graph
} // namespace smtk

#endif
