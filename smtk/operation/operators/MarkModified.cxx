//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/operators/MarkModified.h"

#include "smtk/operation/MarkModified_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace operation
{

MarkModified::Result MarkModified::operateInternal()
{
  auto assocs = this->parameters()->associations();
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");

  // TODO: For now, the associations must be components whose resources should be
  //       marked modified. In the future, once "modified" becomes a ReferenceItem
  //       instead of a ComponentItem, we can directly associate resources.
  smtk::resource::Component::Ptr assoc;
  for (size_t ii = 0; ii < assocs->numberOfValues(); ++ii)
  {
    assoc = std::dynamic_pointer_cast<smtk::resource::Component>(assocs->objectValue(ii));
    if (assoc)
    {
      bool didAppend = modified->appendValue(assoc);
      (void)didAppend;
    }
  }

  // Simply returning the result with modified components is enough because
  // we do not override the default markModifiedResources() implementation
  // which examines "modified" and marks the resources for us.
  return result;
}

const char* MarkModified::xmlDescription() const
{
  return MarkModified_xml;
}
}
}
