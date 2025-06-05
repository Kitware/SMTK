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
#include "smtk/string/json/DeserializationContext.h"
#include "smtk/string/json/jsonManager.h"
#include "smtk/string/json/jsonToken.h"
#include "smtk/view/NameManager.h"
#include "smtk/view/json/jsonNameManager.h"

using ArcMap = smtk::graph::ArcMap;
using namespace smtk::string::literals; // for ""_token

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

void to_json(nlohmann::json& jj, const smtk::markup::IdNature& nature)
{
  jj = natureToken(nature).data();
}

void from_json(const nlohmann::json& jj, smtk::markup::IdNature& nature)
{
  auto tt = jj.get<smtk::string::Token>();
  nature = natureEnumerant(tt);
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
  if (image->shapeData())
  {
    jj["point_ids"] = image->pointIds();
    jj["cell_ids"] = image->cellIds();
  }
}

void to_json(json& jj, const smtk::markup::UnstructuredData* unstructuredData)
{
  to_json(jj, static_cast<const smtk::markup::Component*>(unstructuredData));
  if (unstructuredData->shapeData())
  {
    jj["point_ids"] = unstructuredData->pointIds();
    jj["cell_ids"] = unstructuredData->cellIds();
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

void to_json(json& jj, const smtk::markup::Comment* comment)
{
  to_json(jj, static_cast<const smtk::markup::Comment::Superclass*>(comment));
  if (comment->mimetype().valid())
  {
    jj["mime-type"] = comment->mimetype();
  }
  if (comment->data().valid())
  {
    jj["data"] = comment->data();
  }
}

template<typename NodeType>
void from_json(const json& jj, std::shared_ptr<NodeType>& node)
{
  auto& helper = smtk::resource::json::Helper::instance();
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
  auto& helper = smtk::resource::json::Helper::instance();
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

  // Record length unit (if one exists).
  if (!resource->lengthUnit().empty())
  {
    j["length-unit"] = resource->lengthUnit();
  }

  // Record string-token hashes.
  // Some nodes may use string tokens, so we must serialize that map if it exists.
  auto& tokenManager = smtk::string::Token::manager();
  if (!tokenManager.empty())
  {
    j["tokens"] = tokenManager.shared_from_this();
  }

  // Record the name-manager state.
  // This way, when users load the resource and create new components the numbering
  // picks up where it left off.
  auto nameManager =
    helper.managers() ? helper.managers()->get<smtk::view::NameManager::Ptr>() : nullptr;
  if (nameManager)
  {
    j["name-manager"] = nameManager;
  }

#if 0
  // TODO: Opportunity to record other application data.
  const auto& managers = smtk::resource::json::Helper::instance().managers();
  if (managers)
  {
    std::cout << "Have managers\n";
  }
#endif
}

void from_json(const nlohmann::json& jj, smtk::markup::Resource::Ptr& resource)
{
  auto& helper = smtk::resource::json::Helper::instance();
  if (!resource)
  {
    resource = std::dynamic_pointer_cast<smtk::markup::Resource>(helper.resource());
    if (!resource)
    {
      resource = smtk::markup::Resource::create();
      smtk::resource::json::Helper::pushInstance(resource);
    }
  }

  // Deserialize string tokens first so they can be referenced consistently.
  // Note the use of DeserializationContext to keep the translation table alive.
  smtk::string::DeserializationContext markupContext(
    smtk::string::Token::manager().shared_from_this(), jj["tokens"]);

  // Deserialize the name-manager counter.
  // We create a name manager if none exists in the common::Managers instance.
  auto jNameManager = jj.find("name-manager");
  if (helper.managers())
  {
    auto nameManager = helper.managers()->get<smtk::view::NameManager::Ptr>();
    if (!nameManager)
    {
      nameManager = smtk::view::NameManager::create();
      helper.managers()->insert(nameManager);
    }
    if (jNameManager != jj.end() && nameManager)
    {
      from_json(*jNameManager, nameManager);
    }
  }

  // Deserialize length unit.
  auto jLengthUnit = jj.find("length-unit");
  if (jLengthUnit != jj.end())
  {
    resource->setLengthUnit(jLengthUnit->get<std::string>());
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
  if (jj.contains("nodes"))
  {
    NodeInitializer<smtk::markup::Resource> initializer(resource.get(), jj.at("nodes"));
    smtk::tuple_evaluate<typename Traits::NodeTypes>(initializer);
  }

  // Finally, wait for threads loading VTK data for node geometry to complete.
  for (auto& work : helper.futures())
  {
    work.wait();
  }
}

} // namespace markup
} // namespace smtk
