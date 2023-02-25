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
  , m_taskManager(new smtk::task::Manager)
{
  // Ensure task types are registered to this instance of the task manager.
  // s_registry = smtk::plugin::addToManagers<smtk::task::Registrar>(m_taskManager);
  smtk::plugin::Manager::instance()->registerPluginsTo(m_taskManager);
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
