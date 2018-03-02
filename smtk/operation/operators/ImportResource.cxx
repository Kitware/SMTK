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

ImportResource::ImportResource()
{
}

bool ImportResource::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To import a resource, we must have an operation manager from which we access
  // specific import operations.
  if (m_manager.expired())
  {
    return false;
  }

  return true;
}

ImportResource::Result ImportResource::operateInternal()
{
  auto manager = this->m_manager.lock();

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

    Operation::Index index;
    smtk::attribute::FileItem::Ptr importerFileItem;
    bool found = false;
    for (const Operation::Index& idx : importerGroup.operations())
    {
      importerFileItem = importerGroup.fileItemForOperation(idx);
      if (!fileItem)
      {
        continue;
      }

      if (static_cast<const smtk::attribute::FileItemDefinition*>(fileItem->definition().get())
            ->isValueValid(filename))
      {
        index = idx;
        found = true;
        break;
      }
    }

    if (!found)
    {
      smtkErrorMacro(this->log(), "Could not find importer for file \"" << filename << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::operation::Operation::Ptr importOperation = manager->create(index);

    // Set the local importer's filename field.
    importerFileItem->setValue(filename);

    smtk::operation::Operation::Result importOperationResult = importOperation->operate();
    if (importOperationResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      // An error message should already enter the logger from the local
      // operation.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::resource::ResourcePtr resource = importOperationResult->findResource("resource")->value();
    if (resource == nullptr)
    {
      smtkErrorMacro(this->log(), "Error importing file \"" << filename << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
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

  auto fileDef = smtk::attribute::FileItemDefinition::New("filename");
  fileDef->setNumberOfRequiredValues(1);
  fileDef->setShouldExist(true);
  fileDef->setIsExtensible(true);

  {
    auto manager = this->m_manager.lock();
    if (manager != nullptr)
    {
      smtk::operation::ImporterGroup importerGroup(manager);
      std::string fileFilters = "";
      for (const Operation::Index& index : importerGroup.operations())
      {
        auto fileItem = importerGroup.fileItemForOperation(index);
        if (!fileItem)
        {
          continue;
        }

        if (!fileFilters.empty())
        {
          fileFilters.append(";;");
        }
        fileFilters.append(
          static_cast<const smtk::attribute::FileItemDefinition*>(fileItem->definition().get())
            ->getFileFilters());
      }
      fileDef->setFileFilters(fileFilters);
    }
  }

  opDef->addItemDefinition(fileDef);

  auto resultDef = spec->createDefinition("result(import resource operation)", "result");

  auto resourceDef = smtk::attribute::ResourceItemDefinition::New("resource");
  resourceDef->setNumberOfRequiredValues(0);
  resourceDef->setIsExtensible(true);
  resultDef->addItemDefinition(resourceDef);

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
}
}
