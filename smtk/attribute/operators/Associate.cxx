//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Associate.h"

#include "smtk/attribute/operators/Associate_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace attribute
{

Associate::Result Associate::operateInternal()
{
  // Access the attribute resource to associate.
  smtk::attribute::Resource::Ptr resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
    this->parameters()->associations()->value());

  // Currently, we cannot specialize associations between resources and
  // components. This means an attribute component is allowed as an input. If we
  // recieve an attribute component, access its resource.
  if (resource == nullptr)
  {
    smtk::attribute::Attribute::Ptr attribute =
      std::dynamic_pointer_cast<smtk::attribute::Attribute>(
        this->parameters()->associations()->value());

    if (attribute != nullptr)
    {
      resource = attribute->attributeResource();
    }
  }

  // If we still do not have a valid resource, return with failure.
  if (resource == nullptr)
  {
    smtkErrorMacro(this->log(), "Could not access attribute resource.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the resource to which we will associate.
  auto associateToItem = this->parameters()->findResource("associate to");

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  for (std::size_t i = 0; i < associateToItem->numberOfValues(); i++)
  {
    smtk::resource::Resource::Ptr associated =
      std::dynamic_pointer_cast<smtk::resource::Resource>(associateToItem->value());

    // Associate the resource to the attribute resource.
    bool success = resource->associate(associated);
    if (success)
    {
      result->findResource("resource")->appendValue(resource);
    }
    else
    {
      result->findInt("outcome")->setValue(
        static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
    }
  }

  return result;
}

const char* Associate::xmlDescription() const
{
  return Associate_xml;
}
} // namespace attribute
} // namespace smtk
