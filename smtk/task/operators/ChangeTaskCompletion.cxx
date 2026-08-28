//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/ChangeTaskCompletion.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/json/jsonProject.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/operators/ChangeTaskCompletion_xml.h"

#include <string>

namespace smtk
{
namespace task
{

ChangeTaskCompletion::Result ChangeTaskCompletion::operateInternal()
{
  auto task = this->parameters()->associations()->valueAs<smtk::task::Task>();
  auto project = task ? dynamic_pointer_cast<smtk::project::Project>(task->resource())
                      : smtk::project::Project::Ptr();
  if (!project)
  {
    smtkErrorMacro(log(), "Associated task was null or had no parent project.");
    return this->createResult(Outcome::FAILED);
  }

  auto completedItem = this->parameters()->findAs<smtk::attribute::VoidItem>("completed");
  bool didChange = task->markCompleted(completedItem->isEnabled());

  Result result = this->createResult(
    didChange ? smtk::operation::Operation::Outcome::SUCCEEDED
              : smtk::operation::Operation::Outcome::FAILED);
  if (didChange)
  {
    // Indicate that task-components have been modified:
    auto modified = result->findComponent("modified");
    modified->appendValue(task->shared_from_this());
  }

  return result;
}

void ChangeTaskCompletion::setTask(const smtk::resource::Component::Ptr& task)
{
  this->parameters()->associations()->setValue(task);
}

void ChangeTaskCompletion::setCompleted(bool taskCompleted)
{
  auto completedItem = this->parameters()->findAs<smtk::attribute::VoidItem>("completed");
  completedItem->setIsEnabled(taskCompleted);
}

const char* ChangeTaskCompletion::xmlDescription() const
{
  return ChangeTaskCompletion_xml;
}

} // namespace task
} // namespace smtk
