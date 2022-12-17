//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Add.h"

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

#include "smtk/project/operators/Add_xml.h"

#include <sstream>

namespace smtk
{
namespace project
{

bool Add::configure(
  const smtk::attribute::AttributePtr&,
  const smtk::attribute::ItemPtr& changedItem)
{
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  if (!changedItem || changedItem != projectItem || !projectItem->isValid())
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
    for (const auto& type : project->resources().types())
    {
      resourceDef->setAcceptsEntries(type, true);
    }
  }
  else
  {
    resourceDef->setAcceptsEntries(smtk::common::typeName<smtk::resource::Resource>(), true);
    resourceDef->setRejectsEntries(smtk::common::typeName<smtk::project::Project>(), true);
  }

  return true;
}

Add::Result Add::operateInternal()
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

  if (resource->name().empty())
  {
    resource->setName(role);
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Add::xmlDescription() const
{
  return Add_xml;
}
} // namespace project
} // namespace smtk
