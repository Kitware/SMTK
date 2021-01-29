//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/RemoveResource.h"

#include "smtk/operation/RemoveResource_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

RemoveResource::RemoveResource() = default;

bool RemoveResource::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To create a resource, we must have a resource manager that can read
  // resources.
  if (this->resourceManager() == nullptr)
  {
    return false;
  }
  return true;
}

RemoveResource::Result RemoveResource::operateInternal()
{
  // Access the resource manager (provided by the operation manager that created
  // this operation, since we inherit from ResourceManagerOperation).
  auto resourceManager = this->resourceManager();

  // If there is not resource manager, return with failure.
  if (resourceManager == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the associated resources.
  auto params = this->parameters();
  auto resourceItem = this->parameters()->associations();

  // Construct a result object and access its resource item.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // For each resource...
  for (std::size_t i = 0; i < resourceItem->numberOfValues(); i++)
  {
    // ...access the resource...
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(resourceItem->value(i));

    // ...remove it from the manager...
    bool removed = resourceManager->remove(resource);
    if (removed)
    {
      // ...and add it to the result.
      // TODO: the "expunged" item in the result should accept resources.

      resourceManager->visit([&resource](smtk::resource::Resource& rsrc) {
        rsrc.links().removeAllLinksTo(resource);
        return smtk::common::Processing::CONTINUE;
      });
    }
    else
    {
      // If the resource was not removed, change the result status to failure.
      result->findInt("outcome")->setValue(
        static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
    }
  }

  return result;
}

const char* RemoveResource::xmlDescription() const
{
  return RemoveResource_xml;
}
}
}
