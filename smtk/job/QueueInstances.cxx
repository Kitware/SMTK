//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/QueueInstances.h"

namespace smtk
{
namespace job
{

QueueInstances::QueueInstances(smtk::job::Manager* jobManager)
  : m_manager(jobManager)
{
}

bool QueueInstances::manage(const std::shared_ptr<Queue>& instance)
{
  if (this->Superclass::manage(instance))
  {
    instance->setJobManager(m_manager);
    return true;
  }
  return false;
}

Queue* QueueInstances::findByName(const std::string& name)
{
  Queue* result = nullptr;
  this->visit([&result, name](const std::shared_ptr<Queue>& definition) {
    if (definition->name() == name)
    {
      result = definition.get();
      return smtk::common::Visit::Halt;
    }
    return smtk::common::Visit::Continue;
  });
  return result;
}

} // namespace job
} // namespace smtk
