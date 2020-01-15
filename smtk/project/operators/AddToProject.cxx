//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/AddToProject.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/project/Manager.h"

#include "smtk/project/AddToProject_xml.h"

#include <sstream>

namespace smtk
{
namespace project
{

bool AddToProject::configure(
  const smtk::attribute::AttributePtr&,
  const smtk::attribute::ItemPtr& changedItem)
{
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  if (!changedItem || changedItem != projectItem || projectItem->numberOfValues() != 1)
  {
    return false;
  }

  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  smtk::attribute::ResourceItem::Ptr resourceItem = this->parameters()->findResource("resource");
  resourceItem->reset();

  auto resourceDef = std::const_pointer_cast<smtk::attribute::ResourceItemDefinition>(
    resourceItem->definitionAs<smtk::attribute::ResourceItemDefinition>());

  resourceDef->clearAcceptableEntries();

  if (!project->resources().types().empty())
  {
    for (auto& type : project->resources().types())
    {
      resourceDef->setAcceptsEntries(type, true);
    }
  }
  else
  {
    resourceDef->setAcceptsEntries(smtk::common::typeName<smtk::resource::Resource>(), true);
  }

  return true;
}

AddToProject::Result AddToProject::operateInternal()
{
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  smtk::attribute::ResourceItem::Ptr resourceItem = this->parameters()->findResource("resource");
  smtk::resource::ResourcePtr resource = resourceItem->valueAs<smtk::resource::Resource>();

  std::string role;
  {
    smtk::attribute::StringItem::Ptr roleItem = this->parameters()->findString("role");
    role = roleItem->value();
  }

  if (!project->resources().add(resource, role))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* AddToProject::xmlDescription() const
{
  return AddToProject_xml;
}
} // namespace project
} // namespace smtk
