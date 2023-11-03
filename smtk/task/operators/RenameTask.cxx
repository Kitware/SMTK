//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/RenameTask.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/json/jsonProject.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/operators/RenameTask_xml.h"

#include <string>

namespace smtk
{
namespace task
{

RenameTask::Result RenameTask::operateInternal()
{
  auto task = this->parameters()->associations()->valueAs<smtk::task::Task>();
  auto project = task ? dynamic_pointer_cast<smtk::project::Project>(task->resource())
                      : smtk::project::Project::Ptr();
  if (!project)
  {
    smtkErrorMacro(log(), "Associated task was null or had no parent project.");
    return this->createResult(Outcome::FAILED);
  }

  auto renameItem = this->parameters()->findAs<smtk::attribute::StringItem>("name");
  task->setName(renameItem->value());

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  {
    // Indicate that task-components have been modified:
    auto modified = result->findComponent("modified");
    modified->appendValue(task->shared_from_this());
  }

  smtk::task::json::Helper::popInstance();

  return result;
}

void RenameTask::setName(std::string& name)
{
  auto renameItem = this->parameters()->findAs<smtk::attribute::StringItem>("name");
  renameItem->setValue(name);
}

void RenameTask::setTask(const Task::Ptr& task)
{
  this->parameters()->associations()->setValue(task);
}

const char* RenameTask::xmlDescription() const
{
  return RenameTask_xml;
}

} // namespace task
} // namespace smtk
