//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/Ungroup.h"

#include "smtk/markup/Group.h"
#include "smtk/markup/operators/Delete.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/markup/operators/Ungroup_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace markup
{

Ungroup::Result Ungroup::operateInternal()
{
  Ungroup::Result result;
  auto params = this->parameters();
  auto groups = params->associations()->as<std::set<smtk::markup::Group::Ptr>>(
    [](const resource::PersistentObject::Ptr& obj) {
      return std::dynamic_pointer_cast<smtk::markup::Group>(obj);
    });
  if (groups.empty())
  {
    smtkErrorMacro(this->log(), "No groups to undo.");
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
    return result;
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");

  auto deleteOp = smtk::markup::Delete::create();
  for (const auto& group : groups)
  {
    // Record the members being ungrouped so observers know to display them ungrouped.
    group->members().visit([&modified](const Component* constMember) {
      auto* member = const_cast<Component*>(constMember);
      modified->appendValue(member->shared_from_this());
    });
    // Remove all arcs in/out of the group.
    group->disconnect();
    deleteOp->parameters()->associations()->appendValue(group);
  }
  auto deleteResult = deleteOp->operate(this->childKey());
  auto deleteExpunged = deleteResult->findComponent("expunged");
  for (std::size_t ii = 0; ii < deleteExpunged->numberOfValues(); ++ii)
  {
    expunged->appendValue(deleteExpunged->value(ii));
  }

  return result;
}

const char* Ungroup::xmlDescription() const
{
  return Ungroup_xml;
}

} // namespace markup
} // namespace smtk
