//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/Project.h"

#include "smtk/resource/Manager.h"

#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"

#include "smtk/plugin/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/task/Registrar.h"

namespace smtk
{
namespace project
{
Project::Project(const std::string& typeName)
  : m_resources(this, smtk::resource::Resource::m_manager)
  , m_operations(std::weak_ptr<smtk::operation::Manager>())
  , m_typeName(typeName)
  , m_taskManager(new smtk::task::Manager(this))
{
  // Ensure task types are registered to this instance of the task manager.
  // s_registry = smtk::plugin::addToManagers<smtk::task::Registrar>(m_taskManager);
  smtk::plugin::Manager::instance()->registerPluginsTo(m_taskManager);
}

std::shared_ptr<smtk::project::Project> Project::create(const std::string& typeName)
{
  // No operation manager; try this although it will cause trouble because
  // normally the "Create" operation's result-observer will add it to the
  // project manager.
  auto project = smtk::shared_ptr<smtk::project::Project>(new smtk::project::Project(typeName));
  return project;
}

std::function<bool(const smtk::resource::Component&)> Project::queryOperation(
  const std::string& query) const
{
  if (query.empty())
  {
    // An empty query matches no component.
    // This is so that when we allow joint resource + component queries, there
    // is a way to select the resource (i.e., by providing only a resource clause
    // and an empty component clause).
    return [](const smtk::resource::Component&) { return false; };
  }
  if (query == "*" || query == "any")
  {
    // Match all components.
    return [](const smtk::resource::Component&) { return true; };
  }
  // Behave like our base Resource class for now.
  return this->Resource::queryOperation(query);
}

void Project::visit(smtk::resource::Component::Visitor& visitor) const
{
  if (!visitor)
  {
    return;
  }

  // Currently, a project's only components are tasks.
  m_taskManager->taskInstances().visit([&visitor](const smtk::task::Task::Ptr& task) {
    visitor(task);
    return smtk::common::Visit::Continue;
  });
}

smtk::resource::ComponentPtr Project::find(const smtk::common::UUID& compId) const
{
  // Currently, a project's only components are tasks.
  auto task = m_taskManager->taskInstances().findById(compId);
  return task;
}

bool Project::clean() const
{
  // Check my flag first
  if (!smtk::resource::Resource::clean())
  {
    return false;
  }

  // Check member resources
  for (auto iter = m_resources.begin(); iter != m_resources.end(); ++iter)
  {
    auto resource = *iter;
    if (!resource->clean())
    {
      return false;
    }
  }

  // TODO: Remove tasks? Reset their status?

  return true; // everything in clean state
}
} // namespace project
} // namespace smtk
