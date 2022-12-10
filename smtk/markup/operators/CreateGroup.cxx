//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/CreateGroup.h"

#include "smtk/markup/Group.h"
#include "smtk/markup/operators/CreateGroup_xml.h"

#include "smtk/view/NameManager.h"

#include "smtk/operation/Hints.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

using namespace smtk::model;

namespace smtk
{
namespace markup
{

CreateGroup::Result CreateGroup::operateInternal()
{
  auto params = this->parameters();
  auto assoc = params->associations();
  std::set<smtk::markup::Component::Ptr> members;
  smtk::markup::Resource* resource = nullptr;
  for (std::size_t ii = 0; ii < assoc->numberOfValues(); ++ii)
  {
    if (assoc->isSet(ii))
    {
      auto item = std::dynamic_pointer_cast<smtk::markup::Component>(assoc->value(ii));
      if (item)
      {
        if (!resource)
        {
          resource = dynamic_cast<smtk::markup::Resource*>(item->parentResource());
        }
        members.insert(item);
      }
    }
  }
  CreateGroup::Result result;
  if (!resource || members.empty())
  {
    smtkErrorMacro(this->log(), "No components to group or no components had parent resource.");
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  else
  {
    auto group = resource->createNode<smtk::markup::Group>();
    for (const auto& member : members)
    {
      group->outgoing<arcs::GroupsToMembers>().connect(member);
    }
    auto nameItem = params->findString("name");
    bool haveName = false;
    if (nameItem && nameItem->isEnabled() && !nameItem->isUsingDefault())
    {
      group->setName(params->findString("name")->value());
      haveName = true;
    }
    else if (this->managers())
    {
      if (this->managers()->contains<smtk::view::NameManager::Ptr>())
      {
        auto nameManager = this->managers()->get<smtk::view::NameManager::Ptr>();
        group->setName(nameManager->nameForObject(*group));
        haveName = true;
      }
    }
    if (!haveName && nameItem)
    {
      group->setName(nameItem->defaultValue());
    }
    result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
    result->findComponent("created")->appendValue(group);
    // Groups have a footprint, but not geometry themselves.
    // smtk::operation::MarkGeometry().markModified(group);

    // Create a hint to select and focus on (and eventually rename) the created group.
    smtk::operation::addSelectionHint(
      result, std::set<smtk::resource::PersistentObject::Ptr>{ { group } });
    smtk::operation::addBrowserScrollHint(
      result, std::set<smtk::resource::PersistentObject::Ptr>{ { group } });
  }
  return result;
}

const char* CreateGroup::xmlDescription() const
{
  return CreateGroup_xml;
}

} // namespace markup
} // namespace smtk
