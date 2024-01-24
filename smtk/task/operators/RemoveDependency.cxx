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
  // First, accumulate task pairs while validating that all dependencies exist.
  auto endpoints = this->parameters()->findGroup("task endpoints");
  std::size_t nn = endpoints->numberOfGroups();
  std::multimap<smtk::task::Task::Ptr, smtk::task::Task::Ptr> taskPairs;
  std::size_t invalid = 0;
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    // clang-format off
    auto fromTask = endpoints->findAs<smtk::attribute::ReferenceItem>(ii, "from")->valueAs<smtk::task::Task>();
    auto toTask   = endpoints->findAs<smtk::attribute::ReferenceItem>(ii,   "to")->valueAs<smtk::task::Task>();
    // clang-format on

    auto deps = toTask->dependencies();
    if (deps.find(fromTask) == deps.end())
    {
      ++invalid;
    }
    else
    {
      taskPairs.insert(std::make_pair(toTask, fromTask));
    }
  }
  // Do not remove any dependencies if any task-pairs were missing their dependency.
  if (invalid > 0 || taskPairs.empty())
  {
    smtkErrorMacro(
      this->log(), "Attempt to remove " << invalid << " non-existent dependencies. Aborting.");
    return this->createResult(Outcome::FAILED);
  }

  // The operation should succeed; proceed to remove dependencies.
  auto result = this->createResult(Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");
  for (const auto& entry : taskPairs)
  {
    if (!entry.first->removeDependency(entry.second))
    {
      smtkErrorMacro(
        log(),
        "Failed to remove dependency between \"" << entry.second->name()
                                                 << "\" "
                                                    "and \""
                                                 << entry.first->name() << "\".");
      result->findInt("outcome")->setValue(static_cast<int>(Outcome::FAILED));
    }
    else
    {
      modified->appendValue(entry.second);
      modified->appendValue(entry.first);
    }
  }
  return result;
}

const char* RemoveDependency::xmlDescription() const
{
  return RemoveDependency_xml;
}

} // namespace task
} // namespace smtk
