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

#include "smtk/operation/operators/RemoveResource_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

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
  // this operation
  auto resourceManager = this->resourceManager();

  // If there is not resource manager, return with failure.
  if (resourceManager == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the associated resources.
  auto params = this->parameters();
  auto assoc = this->parameters()->associations();
  bool removeLinks = this->parameters()->findVoid("removeAssociations")->isEnabled();

  // Construct a result object and access its resource item.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto resourceItem = result->findResource("resourcesToExpunge");
  result->findAs<smtk::attribute::VoidItem>("removeAssociations", smtk::attribute::RECURSIVE)
    ->setIsEnabled(removeLinks);

  // For each resource...
  for (std::size_t i = 0; i < assoc->numberOfValues(); i++)
  {
    // ...access the resource...
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(assoc->value(i));

    // append to the to-be-expunged list. smtk::operation::Operation takes care of removal.
    if (resource)
    {
      resourceItem->appendValue(resource);
      if (removeLinks)
      {
        this->resourceManager()->visit([&resource](smtk::resource::Resource& rsrc) {
          rsrc.links().removeAllLinksTo(resource);
          return smtk::common::Processing::CONTINUE;
        });
      }
    }
  }

  return result;
}

const char* RemoveResource::xmlDescription() const
{
  return RemoveResource_xml;
}
} // namespace operation
} // namespace smtk
