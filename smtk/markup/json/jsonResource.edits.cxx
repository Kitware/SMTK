//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/json/jsonResource.h"

#include "smtk/markup/Component.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/detail/NodeContainer.h"
#include "smtk/markup/json/DomainDeserializer.h"
#include "smtk/markup/json/DomainSerializer.h"
#include "smtk/markup/json/NodeInitializer.h"

#include "smtk/common/json/jsonTypeMap.h"
#include "smtk/graph/json/jsonResource.h"
#include "smtk/resource/json/Helper.h"
#include "smtk/resource/json/jsonResource.h"
#include "smtk/string/Manager.h"
#include "smtk/string/Token.h"
#include "smtk/string/json/jsonManager.h"
#include "smtk/string/json/jsonToken.h"

using ArcMap = smtk::graph::ArcMap;
using namespace smtk::string;

namespace smtk
{
namespace markup
{

void to_json(json& jj, const smtk::markup::Domain* domain)
{
  if (!domain)
  {
    throw std::invalid_argument("Null domain.");
  }
  jj["name"] = domain->name();
}

void to_json(json& jj, const smtk::markup::IdSpace* idSpace)
{
  if (!idSpace)
  {
    throw std::invalid_argument("Null id-space.");
  }
  to_json(jj, static_cast<const smtk::markup::Domain*>(idSpace));
  jj["range"] = idSpace->range();
  // TODO: Iterate over the interval tree of assigned IDs and
  //       generate map from (domain) IDs to component UUIDs
}

void to_json(nlohmann::json& jj, const smtk::markup::Nature& nature)
{
  switch (nature)
  {
    case Nature::Primary:
      jj = "primary";
      break;
    case Nature::Referential:
      jj = "referential";
      break;
    case Nature::NonExclusive:
      jj = "non_exclusive";
      break;
    case Nature::Unassigned:
      jj = "unassigned";
      break;
  }
}

void from_json(const nlohmann::json& jj, smtk::markup::Nature& nature)
{
  auto tt = jj.get<std::string>();
  if (tt == "primary")
  {
    nature = Nature::Primary;
  }
  else if (tt == "referential")
  {
    nature = Nature::Referential;
  }
  else if (tt == "non_exclusive")
  {
    nature = Nature::NonExclusive;
  }
  else // (tt == "unassigned")
  {
    nature = Nature::Unassigned;
  }
  /* Once the ""_token operator is constexpr, we can do this instead:
  switch (tt.id())
  {
  case "primary"_token: nature = Nature::Primary; break;
  case "referential"_token: nature = Nature::Referential; break
  case "non_exclusive"_token: nature = Nature::NonExclusive; break;
  default: // fall through
  case "unassigned"_token: nature =  Nature::Unassigned; break;
  }
  */
}

void to_json(json& jj, const smtk::markup::AssignedIds& assignedIds)
{
  jj["nature"] = assignedIds.nature();
  jj["range"] = assignedIds.range();
  if (auto space = assignedIds.space())
  {
    jj["space"] = space->name();
  }
}

void to_json(json& jj, const smtk::markup::Component* component)
{
  if (!component)
  {
    throw std::invalid_argument("Null component");
  }
  jj["id"] = component->id();
  if (!component->name().empty())
  {
    jj["name"] = component->name();
  }
}

void to_json(json& jj, const smtk::markup::Box* box)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(box));
  jj["lo"] = box->range()[0];
  jj["hi"] = box->range()[1];
}

void to_json(json& jj, const smtk::markup::Field* field)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(field));
  jj["field_type"] = field->fieldType();
}

void to_json(json& jj, const smtk::markup::ImageData* image)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(image));
  jj["point_ids"] = image->pointIds();
  jj["cell_ids"] = image->cellIds();
  if (const auto* imageURL = image->incoming<URLsToData>().node())
  {
    jj["shape"] = imageURL->location();
  }
}

void to_json(json& jj, const smtk::markup::UnstructuredData* unstructuredData)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(unstructuredData));
  jj["point_ids"] = unstructuredData->pointIds();
  jj["cell_ids"] = unstructuredData->cellIds();
  if (const auto* meshURL = unstructuredData->incoming<URLsToData>().node())
  {
    jj["shape"] = meshURL->location();
  }
}

void to_json(json& jj, const smtk::markup::Sphere* sphere)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(sphere));
  jj["center"] = sphere->center();
  jj["radius"] = sphere->radius();
}

void to_json(json& jj, const smtk::markup::OntologyIdentifier* oid)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(oid));
  jj["ontology_id"] = oid->ontologyId();
}

template<typename NodeType>
void from_json(const json& jj, std::shared_ptr<NodeType>& node)
{
  auto helper = smtk::resource::json::Helper::instance();
  auto resource = std::dynamic_pointer_cast<smtk::graph::ResourceBase>(helper.resource());
  if (resource)
  {
    // Construct a node of the proper type with its resource and UUID set.
    // Note that you must provide a constructor that passes these arguments
    // to the base graph-resource component class or you will have build errors.
    node = std::make_shared<NodeType>(resource, jj["id"].get<smtk::common::UUID>());
    // Adding the node can fail if the node's type is disallowed by the resource.
    if (!resource->addNode(node))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not add node.");
    }
    // TODO: Setting the name after insertion works but could be slow, since
    // for multi-index containers the node must effectively be removed and
    // then re-inserted to change the name. However, setting the name before
    // adding the component fails because the container hasn't indexed it yet.
    auto it = jj.find("name");
    if (it != jj.end())
    {
      node->setName(it->get<std::string>());
    }
  }
}

template<>
void from_json<BoundaryOperator>(const json& jj, std::shared_ptr<BoundaryOperator>& domain)
{
  domain = std::make_shared<BoundaryOperator>(jj);
}

template<>
void from_json<IdSpace>(const json& jj, std::shared_ptr<IdSpace>& domain)
{
  domain = std::make_shared<IdSpace>(jj);
}

template<>
void from_json<Index>(const json& jj, std::shared_ptr<Index>& domain)
{
  domain = std::make_shared<Index>(jj);
}

template<>
void from_json<ParameterSpace>(const json& jj, std::shared_ptr<ParameterSpace>& domain)
{
  domain = std::make_shared<ParameterSpace>(jj);
}

void to_json(nlohmann::json& j, const smtk::markup::Resource::Ptr& resource)
{
  // Add version number and other information inherited from parent.
  smtk::graph::to_json(j, std::static_pointer_cast<smtk::markup::GraphResource>(resource));

  // Record domains.
  json domainData;
  DomainSerializer<smtk::markup::Resource> domainSerializer(resource.get(), domainData);
  smtk::tuple_evaluate<typename Traits::DomainTypes>(domainSerializer);
  if (!domainData.empty())
  {
    j["domains"] = domainData;
  }

  // Record string-token hashes.
  // Some nodes may use string tokens, so we must serialize that map if it exists.
  auto& tokenManager = smtk::string::Token::manager();
  if (!tokenManager.empty())
  {
    j["tokens"] = tokenManager.shared_from_this();
  }

  // TODO: Opportunity to record other application data.
  const auto& managers = smtk::resource::json::Helper::instance().managers();
  if (managers)
  {
    std::cout << "Have managers\n";
  }
}

void from_json(const nlohmann::json& jj, smtk::markup::Resource::Ptr& resource)
{
  if (!resource)
  {
    resource = std::dynamic_pointer_cast<smtk::markup::Resource>(
      smtk::resource::json::Helper::instance().resource());
    if (!resource)
    {
      resource = smtk::markup::Resource::create();
      smtk::resource::json::Helper::pushInstance(resource);
    }
  }

  // Deserialize string tokens first so they can be referenced consistently.
  auto jTokens = jj.find("tokens");
  if (jTokens != jj.end())
  {
    auto& tokenManager = smtk::string::Token::manager();
    tokenManager.shared_from_this() = *jTokens;
  }

  // Deserialize domains
  auto jDomains = jj.find("domains");
  if (jDomains != jj.end())
  {
    DomainDeserializer<smtk::markup::Resource> domainDeserializer(resource.get(), *jDomains);
    smtk::tuple_evaluate<typename Traits::DomainTypes>(domainDeserializer);
  }

  // Deserialize arcs and nodes using smtk::graph::from_json()
  auto tmp = std::static_pointer_cast<smtk::markup::GraphResource>(resource);
  smtk::graph::from_json(jj, tmp);

  // Now initialize nodes from their JSON.
  NodeInitializer<smtk::markup::Resource> initializer(resource.get(), jj.at("nodes"));
  smtk::tuple_evaluate<typename Traits::NodeTypes>(initializer);
}

} // namespace markup
} // namespace smtk
