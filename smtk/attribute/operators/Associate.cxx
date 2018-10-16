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

#include "smtk/attribute/Associate_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

namespace smtk
{
namespace attribute
{

Associate::Result Associate::operateInternal()
{
  // Access the attribute resource to associate.
  smtk::attribute::Resource::Ptr resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
    this->parameters()->associations()->objectValue());

  // Access the resource to which we will associate.
  auto associateToItem = this->parameters()->findResource("associate to");

  bool success = true;

  for (std::size_t i = 0; i < associateToItem->numberOfValues(); i++)
  {
    smtk::resource::Resource::Ptr associated =
      std::dynamic_pointer_cast<smtk::resource::Resource>(associateToItem->objectValue());

    // Associate the resource to the attribute resource.
    success &= resource->associate(associated);
  }

  return (success ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
                  : this->createResult(smtk::operation::Operation::Outcome::FAILED));
}

const char* Associate::xmlDescription() const
{
  return Associate_xml;
}
}
}
