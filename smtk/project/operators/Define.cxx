//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Define.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/project/Manager.h"

#include "smtk/project/operators/Define_xml.h"

#include <sstream>

namespace
{
struct ResourceMetadataKeyContainer
{
  ResourceMetadataKeyContainer(smtk::resource::MetadataObservers::Key&& key)
    : m_key(std::move(key))
  {
  }

  smtk::resource::MetadataObservers::Key m_key;
};

struct OperationMetadataKeyContainer
{
  OperationMetadataKeyContainer(smtk::operation::MetadataObservers::Key&& key)
    : m_key(std::move(key))
  {
  }

  smtk::operation::MetadataObservers::Key m_key;
};
} // namespace

namespace nlohmann
{
namespace detail
{
template<>
struct has_to_json<
  nlohmann::json,
  std::unordered_map<smtk::common::UUID, ResourceMetadataKeyContainer>> : std::false_type
{
};
template<>
struct has_to_json<
  nlohmann::json,
  std::unordered_map<smtk::common::UUID, OperationMetadataKeyContainer>> : std::false_type
{
};
} // namespace detail
} // namespace nlohmann

namespace smtk
{
namespace project
{

Define::Result Define::operateInternal()
{
  std::string name;
  {
    smtk::attribute::StringItem::Ptr nameItem = this->parameters()->findString("name");
    name = nameItem->value();
  }

  std::set<std::string> resources;
  {
    // Get the available resources list
    smtk::attribute::StringItem::Ptr resourcesItem = this->parameters()->findString("resources");
    if (resourcesItem->isEnabled())
    {
      for (std::size_t i = 0; i < resourcesItem->numberOfValues(); ++i)
      {
        std::stringstream s;
        s << "Adding " << resourcesItem->value(i);
        smtkInfoMacro(this->log(), s.str());
        resources.insert(resourcesItem->value(i));
      }
    }
  }

  std::set<std::string> operations;
  {
    // Get the available operations list
    smtk::attribute::StringItem::Ptr operationsItem = this->parameters()->findString("operations");
    if (operationsItem->isEnabled())
    {
      for (std::size_t i = 0; i < operationsItem->numberOfValues(); ++i)
      {
        std::stringstream s;
        s << "Adding " << operationsItem->value(i);
        smtkInfoMacro(this->log(), s.str());
        operations.insert(operationsItem->value(i));
      }
    }
  }

  if (!this->projectManager()->registerProject(name, resources, operations))
  {
    smtkErrorMacro(this->log(), "Cannot define project type \"" << name << "\"");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

Define::Specification Define::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto defineDef = spec->findDefinition("define");

  auto projectManager = this->projectManager();
  if (!projectManager)
  {
    return spec;
  }

  smtk::attribute::StringItemDefinitionPtr resourcesDef;
  {
    std::vector<smtk::attribute::StringItemDefinition::Ptr> stringItemDefinitions;
    auto stringItemDefinitionFilter = [](smtk::attribute::StringItemDefinition::Ptr ptr) {
      return ptr->name() == "resources";
    };
    defineDef->filterItemDefinitions(stringItemDefinitions, stringItemDefinitionFilter);
    resourcesDef = stringItemDefinitions[0];
  }

  resourcesDef->setIsExtensible(true);
  resourcesDef->clearDiscreteValues();
  for (const auto& metadatum : projectManager->resourceManager()->metadata())
  {
    resourcesDef->addDiscreteValue(metadatum.typeName());
  }

  spec->properties().insertPropertyType<ResourceMetadataKeyContainer>();

  spec->properties().emplace<ResourceMetadataKeyContainer>(
    "update_resource_list",
    projectManager->resourceManager()->metadataObservers().insert(
      [resourcesDef](const smtk::resource::Metadata& md, bool adding) {
        if (!adding)
        {
          return;
        }

        resourcesDef->addDiscreteValue(md.typeName());
      },
      "Define: Update resources list when new resource types are added"));

  smtk::attribute::StringItemDefinitionPtr operationsDef;
  {
    std::vector<smtk::attribute::StringItemDefinition::Ptr> stringItemDefinitions;
    auto stringItemDefinitionFilter = [](smtk::attribute::StringItemDefinition::Ptr ptr) {
      return ptr->name() == "operations";
    };
    defineDef->filterItemDefinitions(stringItemDefinitions, stringItemDefinitionFilter);
    operationsDef = stringItemDefinitions[0];
  }

  operationsDef->setIsExtensible(true);
  operationsDef->clearDiscreteValues();

  spec->properties().insertPropertyType<OperationMetadataKeyContainer>();

  spec->properties().emplace<OperationMetadataKeyContainer>(
    "update_operation_list",
    projectManager->operationManager()->metadataObservers().insert(
      [operationsDef](const smtk::operation::Metadata& md, bool adding) {
        if (!adding)
        {
          return;
        }

        operationsDef->addDiscreteValue(md.typeName());
      },
      "Define: Update operations list when new operation types are added"));

  return spec;
}

const char* Define::xmlDescription() const
{
  return Define_xml;
}
} // namespace project
} // namespace smtk
