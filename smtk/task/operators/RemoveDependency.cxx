//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/RemoveDependency.h"

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

#include "smtk/task/operators/RemoveDependency_xml.h"

#include <string>

namespace smtk
{
namespace task
{

RemoveDependency::Result RemoveDependency::operateInternal()
{
  auto fromTask = this->parameters()->associations()->valueAs<smtk::task::Task>();
  auto toTask = this->parameters()->findComponent("to")->valueAs<smtk::task::Task>();
  if (!fromTask || !toTask)
  {
    smtkErrorMacro(log(), "Associated or referenced task was null or of wrong type.");
    return this->createResult(Outcome::FAILED);
  }

  if (!toTask->removeDependency(fromTask))
  {
    smtkErrorMacro(log(), "Dependency did not exist.");
    return this->createResult(Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  {
    auto modified = result->findComponent("modified");

    // Indicate that the tasks have been modified:
    modified->appendValue(fromTask);
    modified->appendValue(toTask);

    // Hint to the application to select and center on deserialized tasks:
    // smtk::operation::addSelectionHint(result, sharedTasks);
    // smtk::operation::addBrowserScrollHint(result, sharedTasks);
  }

  return result;
}

const char* RemoveDependency::xmlDescription() const
{
  return RemoveDependency_xml;
}

} // namespace task
} // namespace smtk
