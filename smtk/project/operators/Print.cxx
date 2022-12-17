//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Print.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/project/Project.h"
#include "smtk/project/ResourceContainer.h"

#include "smtk/project/operators/Print_xml.h"

namespace smtk
{
namespace project
{

Print::Result Print::operateInternal()
{
  // Access the project to write.
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  // Print the project's contents.
  for (const auto& resource : project->resources())
  {
    smtkInfoMacro(
      this->log(),
      "Resource <" << resource->id() << ">\n"
                   << "  name: " << resource->name() << "\n"
                   << "  type: " << resource->typeName() << "\n"
                   << "  role: " << detail::role(resource) << "\n");
  }

  // Return with success.
  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Print::xmlDescription() const
{
  return Print_xml;
}
} // namespace project
} // namespace smtk
