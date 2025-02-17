//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/CreateResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/operation/CreateResource_xml.h"

namespace smtk
{
namespace operation
{

CreateResource::CreateResource() {}

smtk::operation::Operation::Result CreateResource::operateInternal()
{
  auto params = this->parameters();
  auto typeItem = params->findString("type");

  auto resourceManager = this->resourceManager();
  if (!resourceManager)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot resolve resource manager.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");

  for (auto typeIt = typeItem->begin(); typeIt != typeItem->end(); ++typeIt)
  {
    std::string type = *typeIt;

    smtk::resource::Resource::Ptr resource = resourceManager->create(type);
    if (!resource)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Cannot create resource of type " << type << ".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    created->appendValue(resource);
  }

  return result;
}

const char* CreateResource::xmlDescription() const
{
  return CreateResource_xml;
}

void CreateResource::generateSummary(CreateResource::Result& res)
{
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::StringItemPtr sitem = this->parameters()->findString("type");
  std::string label = this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), label << ": created \"" << sitem->value(0) << "\"");
  }
  else
  {
    smtkErrorMacro(this->log(), label << ": failed to create \"" << sitem->value(0) << "\"");
  }
}
} // namespace operation
} // namespace smtk
