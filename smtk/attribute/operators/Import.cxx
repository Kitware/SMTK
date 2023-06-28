//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Import.h"

#include "smtk/attribute/operators/Import_xml.h"

#include "smtk/attribute/AssociationRuleManager.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/EvaluatorManager.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ItemDefinitionManager.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Managers.h"
#include "smtk/common/Paths.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace attribute
{

Import::Result Import::operateInternal()
{
  // Access the file name.
  std::string filename = this->parameters()->findFile("filename")->value();

  // Check whether or not to use directory info
  smtk::attribute::VoidItem::Ptr directoryInfoItem =
    this->parameters()->findVoid("UseDirectoryInfo");
  bool useDirectoryInfo = directoryInfoItem->isEnabled();

  smtk::attribute::ResourcePtr resource;
  // Check if attribute resource is specified
  smtk::attribute::ReferenceItem::Ptr existingResourceItem = this->parameters()->associations();
  if (existingResourceItem->numberOfValues() > 0)
  {
    resource = std::static_pointer_cast<smtk::attribute::Resource>(existingResourceItem->value());
  }
  else
  {
    // Create an attribute resource. If available, use the item definition
    // manager to populate the resource with custom items.
    resource = smtk::attribute::Resource::create();

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

        // ...and that container has an EvaluatorManager...
        if (mgrs->contains<smtk::attribute::EvaluatorManager::Ptr>())
        {
          // ...add evaluators to the newly created attribute resource.
          auto evaluatorManager = mgrs->get<smtk::attribute::EvaluatorManager::Ptr>();
          evaluatorManager->registerEvaluatorsTo(resource);
        }
      }
    }

    auto name = smtk::common::Paths::stem(filename);
    resource->setName(name);
  }

  // Populate the attribute resource with the contents of the file.
  smtk::io::AttributeReader reader;

  // Since we could have multiple operations running at the same time and
  // since the AttributeReader uses logger to track parsing errors
  // we need to create an internal logger and then merge it when the
  // read is completed
  smtk::io::Logger logger;

  if (reader.read(resource, filename, useDirectoryInfo, logger))
  {
    smtkErrorMacro(logger, "Encountered errors while reading \"" << filename << "\".");
    this->log().append(logger);
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  this->log().append(logger);
  // Create a result object.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Populate the result object with the new attribute resource.
  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  // Return with success.
  return result;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}
} // namespace attribute
} // namespace smtk
