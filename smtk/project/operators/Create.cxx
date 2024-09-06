//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Create.h"

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

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

#include "smtk/project/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/project/operators/Create_xml.h"

#include <sstream>

#if NLOHMANN_JSON_VERSION_MAJOR > 3 ||                                                             \
  (NLOHMANN_JSON_VERSION_MAJOR == 3 && NLOHMANN_JSON_VERSION_MINOR >= 11)
#include "nlohmann/detail/abi_macros.hpp"
#else

#define NLOHMANN_JSON_NAMESPACE_BEGIN                                                              \
  namespace nlohmann                                                                               \
  {
#define NLOHMANN_JSON_NAMESPACE_END }
#endif

namespace
{
struct KeyContainer
{
  KeyContainer(smtk::project::MetadataObservers::Key&& key)
    : m_key(std::move(key))
  {
  }

  smtk::project::MetadataObservers::Key m_key;
};
} // namespace

NLOHMANN_JSON_NAMESPACE_BEGIN

namespace detail
{
template<>
struct has_to_json<nlohmann::json, std::unordered_map<smtk::common::UUID, KeyContainer>>
  : std::false_type
{
};
} // namespace detail
NLOHMANN_JSON_NAMESPACE_END

namespace smtk
{
namespace project
{

Create::Result Create::operateInternal()
{
  std::string typeName;
  {
    smtk::attribute::StringItem::Ptr typeNameItem = this->parameters()->findString("typeName");
    typeName = typeNameItem->value();
  }

  auto project = this->projectManager()->create(typeName);

  if (!project)
  {
    smtkErrorMacro(this->log(), "Cannot create project type \"" << typeName << "\"");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  project->resources().setManager(this->managers()->get<smtk::resource::Manager::Ptr>());
  project->operations().setManager(this->managers()->get<smtk::operation::Manager::Ptr>());

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setNumberOfValues(1);
    created->setValue(project);
  }

  return result;
}

Create::Specification Create::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto createDef = spec->findDefinition("create");

  auto projectManager = this->projectManager();
  if (!projectManager)
  {
    return spec;
  }

  smtk::attribute::StringItemDefinitionPtr projectDef;
  {
    std::vector<smtk::attribute::StringItemDefinition::Ptr> stringItemDefinitions;
    auto stringItemDefinitionFilter = [](smtk::attribute::StringItemDefinition::Ptr ptr) {
      return ptr->name() == "typeName";
    };
    createDef->filterItemDefinitions(stringItemDefinitions, stringItemDefinitionFilter);
    projectDef = stringItemDefinitions[0];
  }

  spec->properties().insertPropertyType<KeyContainer>();
  spec->properties().emplace<KeyContainer>(
    "update_project_list",
    projectManager->metadataObservers().insert(
      [projectDef](const smtk::project::Metadata& md, bool adding) {
        if (!adding)
        {
          return;
        }

        projectDef->addDiscreteValue(md.typeName());
      },
      "Create: Update project list when new project types are added"));

  return spec;
}

const char* Create::xmlDescription() const
{
  return Create_xml;
}
} // namespace project
} // namespace smtk
