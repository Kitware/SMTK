//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Read.h"

#include "smtk/attribute/operators/Read_xml.h"

#include "smtk/attribute/AssociationRuleManager.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/EvaluatorManager.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ItemDefinitionManager.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/attribute/json/jsonResource.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/json/Helper.h"

#include "smtk/common/VersionNumber.h"
#include "smtk/common/json/jsonVersionNumber.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

namespace smtk
{
namespace attribute
{

Read::Result Read::operateInternal()
{
  using smtk::common::VersionNumber;

  // Access the file name.
  std::string filename = this->parameters()->findFile("filename")->value();

  // Check the file's validity.
  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(log(), "Cannot read file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Configure the JSON helper with managers passed to us.
  auto& helper = smtk::resource::json::Helper::instance();
  helper.clear();
  helper.setManagers(this->managers());

  // Read the file into nlohmann json.
  nlohmann::json j;
  try
  {
    j = nlohmann::json::parse(file);
  }
  catch (...)
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  bool supportedVersion = false;
  VersionNumber version;
  try
  {
    // Is this a supported version?  Should be 3.0, 4.0, 5.0, 6.0, or 7.0.
    version = j.at("version");
    if (version >= VersionNumber(3) && version < VersionNumber(8))
    {
      supportedVersion = true;
    }
  }
  catch (const std::exception&)
  {
    supportedVersion = false;
  }
  if (!supportedVersion)
  {
    if (!version.isValid())
    {
      smtkErrorMacro(log(), "Cannot read attribute file \"" << filename << "\" - Missing Version.");
    }
    else
    {
      smtkErrorMacro(
        log(),
        "Cannot read attribute file \"" << filename << "\" - Unsupported Version: " << version
                                        << ".");
    }
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create an attribute resource. If available, use the item definition
  // manager to populate the resource with custom items.
  smtk::attribute::Resource::Ptr resource = smtk::attribute::Resource::create();

  // If the operation has an associated manager...
  if (auto mgr = manager())
  {
    // ...and the manager has an associated container of managers...
    if (auto mgrs = mgr->managers())
    {
      // ...and that container has an item definition manager...
      if (mgrs->contains<smtk::attribute::ItemDefinitionManager::Ptr>())
      {
        // ...add custom item definitions to the newly created attribute
        // resource.
        auto itemDefinitionManager = mgrs->get<smtk::attribute::ItemDefinitionManager::Ptr>();
        itemDefinitionManager->registerDefinitionsTo(resource);
      }

      // if that container has an association rule manager...
      if (mgrs->contains<smtk::attribute::AssociationRuleManager::Ptr>())
      {
        // ...add custom association rules to the newly created attribute
        // resource.
        auto associationRuleManager = mgrs->get<smtk::attribute::AssociationRuleManager::Ptr>();
        associationRuleManager->registerRulesTo(resource);
      }

      // ...and that container has an Evaluator...
      if (mgrs->contains<smtk::attribute::EvaluatorManager::Ptr>())
      {
        // ...add evaluators to the newly created attribute resource.
        auto evaluatorManager = mgrs->get<smtk::attribute::EvaluatorManager::Ptr>();
        evaluatorManager->registerEvaluatorsTo(resource);
      }
    }
  }

  // Copy the contents of the json object into the attribute resource.
  smtk::attribute::from_json(j, resource);
  resource->setLocation(filename);

  // Create a result object.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Populate the result object with the new attribute resource.
  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setNumberOfValues(1);
    created->setValue(resource);
  }

  helper.clear();

  // Return with success.
  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

void Read::markModifiedResources(Read::Result& res)
{
  auto resourceItem = res->findResource("resource");

  // Special case, if loaded from project, resource item is not set
  // Reason is not known, but this serves as a workaround
  if ((resourceItem->numberOfValues() == 1) && resourceItem->value(0) == nullptr)
  {
    return;
  }

  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);
    if (resource != nullptr)
    {
      // Set the resource as unmodified from its persistent (i.e. on-disk) state
      resource->setClean(true);
    }
  }
}

smtk::resource::ResourcePtr read(
  const std::string& filename,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Read::Ptr read = Read::create();
  read->setManagers(managers);
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}
} // namespace attribute
} // namespace smtk
