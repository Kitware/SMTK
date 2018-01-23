//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/SaveResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/operation/SaveResource_xml.h"

#include "nlohmann/json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace smtk
{
namespace operation
{

SaveResource::SaveResource()
{
}

bool SaveResource::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To save a resource, we must have a resource manager that can read
  // resources. The resource manager may be associated with the resource itself.
  auto resource = this->parameters()->findResource("resource")->value();
  if (resource->manager() == nullptr && this->resourceManager() == nullptr)
  {
    return false;
  }

  return true;
}

smtk::operation::NewOp::Result SaveResource::operateInternal()
{
  auto params = this->parameters();
  auto fileItem = params->findFile("filename");
  auto setFilename = fileItem->isEnabled();
  auto resourceItem = params->findResource("resource");

  if (resourceItem->numberOfValues() < 1)
  {
    smtkErrorMacro(this->log(), "At least one resource must be selected for saving.");
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  if (setFilename && resourceItem->numberOfValues() != fileItem->numberOfValues())
  {
    smtkErrorMacro(this->log(), "Number of filenames must match number of resources.");
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  int rr = 0;
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit, ++rr)
  {
    auto resource = *rit;
    // First try to use the resource manager associated with the resource.
    auto resourceManager = resource->manager();

    // If the resource has no associated resource manager, that's ok. Try using
    // the resource manager associated to the operator.
    if (!resourceManager)
    {
      resourceManager = this->resourceManager();
    }

    // If neither the operator nor the resource have an associated resource
    // manager, there's not much we can do.
    if (!resourceManager)
    {
      smtkErrorMacro(this->log(), "Resource \"" << resource->uniqueName() << "\" (\""
                                                << resource->location() << "\") has no manager.");
      return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
    }

    auto metadata =
      resourceManager->metadata().get<smtk::resource::IndexTag>().find(resource->index());
    if (metadata == resourceManager->metadata().get<smtk::resource::IndexTag>().end())
    {
      smtkErrorMacro(this->log(), "Resource \""
          << resource->uniqueName() << "\" (\"" << resource->location()
          << "\") is not registered with the available resource manager.");
      return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
    }

    if (!metadata->write)
    {
      smtkErrorMacro(this->log(), "Resource metadata for " << resource->uniqueName()
                                                           << " has a null write method.");
      return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
    }

    if (resource->location().empty())
    {
      auto filename = setFilename ? fileItem->value(rr) : "";
      if (filename.empty())
      {
        smtkErrorMacro(this->log(), "An empty filename is not allowed.");
        return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
      }
      resource->setLocation(filename);
    }

    if (!metadata->write(resource))
    {
      // The writer will have logged an error message.
      return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
    }
  }
  return this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);
}

const char* SaveResource::xmlDescription() const
{
  return SaveResource_xml;
}

void SaveResource::generateSummary(SaveResource::Result& res)
{
  std::ostringstream msg;
  int outcome = res->findInt("outcome")->value();
  auto resourceItem = this->parameters()->findResource("resource");
  msg << this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    msg << ": wrote \"" << resourceItem->value(0)->location() << "\"";
    smtkInfoMacro(this->log(), msg.str());
  }
  else
  {
    msg << ": failed to write \"" << resourceItem->value(0)->location() << "\"";
    smtkErrorMacro(this->log(), msg.str());
  }
}
}
}
