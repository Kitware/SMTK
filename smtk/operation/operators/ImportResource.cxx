//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/ImportResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/groups/ImporterGroup.h"

#include <fstream>

namespace smtk
{
namespace operation
{

ImportResource::ImportResource() = default;

bool ImportResource::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To import a resource, we must have an operation manager from which we access
  // specific import operations.
  auto manager = m_manager.lock();

  if (manager == nullptr)
  {
    return false;
  }

  auto params = this->parameters();
  auto fileItem = params->findFile(ImportResource::file_item_name);

  smtk::operation::ImporterGroup importerGroup(manager);

  for (auto fileIt = fileItem->begin(); fileIt != fileItem->end(); ++fileIt)
  {
    if (importerGroup.operationsForFileName(*fileIt).empty())
    {
      return false;
    }
  }

  return true;
}

ImportResource::Result ImportResource::operateInternal()
{
  auto manager = m_manager.lock();

  if (manager == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto params = this->parameters();
  auto fileItem = params->findFile("filename");

  std::string type;

  smtk::operation::ImporterGroup importerGroup(manager);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  for (auto fileIt = fileItem->begin(); fileIt != fileItem->end(); ++fileIt)
  {
    std::string filename = *fileIt;

    // We take the first operation that accepts the file name. This set is
    // guaranteed to be nonempty because it was checked in ableToOperate, but
    // let's be cautious.
    auto ops = importerGroup.operationsForFileName(filename);
    assert(!ops.empty());
    Operation::Index index = *ops.begin();

    // Create the local operation.
    smtk::operation::Operation::Ptr importOperation = manager->create(index);

    // Access the local operation's file item.
    smtk::attribute::FileItem::Ptr importerFileItem =
      importOperation->parameters()->findFile(importerGroup.fileItemNameForOperation(index));

    // Set the local importer's filename field.
    importerFileItem->setValue(filename);

    smtk::operation::Operation::Result importOperationResult = importOperation->operate();
    if (
      importOperationResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      // An error message should already enter the logger from the local
      // operation.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::resource::ResourcePtr resource =
      importOperationResult->findResource("resourcesCreated")->value();
    if (resource == nullptr)
    {
      smtkErrorMacro(this->log(), "Error importing file \"" << filename << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
  }

  return result;
}

ImportResource::Specification ImportResource::createSpecification()
{
  Specification spec = this->createBaseSpecification();

  auto opDef = spec->createDefinition("import resource op", "operation");
  opDef->setBriefDescription("Import a resource.");

  const char detailedDescription[] =
    "<p>A class for importing resources."
    "<p>Given files, this operation examines each file's extension, identifies"
    "an import operation that accepts files of this type and executes this"
    "operation on the file.";

  opDef->setDetailedDescription(std::string(detailedDescription));

  auto fileDef = smtk::attribute::FileItemDefinition::New(ImportResource::file_item_name);
  fileDef->setNumberOfRequiredValues(1);
  fileDef->setShouldExist(true);
  fileDef->setIsExtensible(true);

  {
    auto manager = m_manager.lock();
    // If there is an available operation manager at the time of creation, then
    // we can query it for file filters. Otherwise, we are forced to accept all
    // files (that's ok, though, because the called filter will still cause the
    // operation to be uncallable).
    if (manager != nullptr)
    {
      std::weak_ptr<smtk::attribute::FileItemDefinition> weakFileItemDefPtr;
      // Define a metadata observer that appends the file filters of an import
      // operation to the file definition.
      auto observer = [&, weakFileItemDefPtr](const smtk::operation::Metadata& md, bool adding) {
        if (!adding)
        {
          return;
        }

        auto fileItemDef = weakFileItemDefPtr.lock();

        // If the file item definition is no longer accessible, there's not much
        // we can do.
        if (fileItemDef == nullptr)
        {
          return;
        }

        // We are only interested in import operations.
        std::set<std::string> groups = md.groups();
        if (groups.find(ImporterGroup::type_name) == groups.end())
        {
          return;
        }

        std::string fileFilters = fileItemDef->getFileFilters();

        // If the operation is registered as an importer, then we must be able
        // to access its file item definition.
        smtk::operation::ImporterGroup importerGroup(manager);
        auto localFileItemDef = importerGroup.fileItemDefinitionForOperation(md.index());
        assert(localFileItemDef != nullptr);

        if (!fileFilters.empty())
        {
          fileFilters.append(";;");
        }
        fileFilters.append(localFileItemDef->getFileFilters());
      };

      // Add this metadata observer to the set of metadata observers,
      // invoking it immediately on all extant metadata.
      manager->metadataObservers().insert(observer);
      std::for_each(
        manager->metadata().begin(), manager->metadata().end(), [&](const Metadata& metadata) {
          observer(metadata, true);
        });
    }
  }

  opDef->addItemDefinition(fileDef);

  auto resultDef = spec->createDefinition("result(import resource operation)", "result");
  return spec;
}

void ImportResource::generateSummary(ImportResource::Result& res)
{
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::FileItemPtr fitem = this->parameters()->findFile("filename");
  std::string label = this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), label << ": imported \"" << fitem->value(0) << "\"");
  }
  else
  {
    smtkErrorMacro(this->log(), label << ": failed to import \"" << fitem->value(0) << "\"");
  }
}
} // namespace operation
} // namespace smtk
