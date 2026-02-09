//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/paraview/job/Registrar.h"

#include "smtk/extension/qt/job/ContainerQueue.h"

#include "smtk/job/Manager.h"
#include "smtk/job/Resource.h"

#include <set>

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace job
{
namespace
{

// Queues this registrar has added to the job manager (for removal upon unregistration).
std::set<std::shared_ptr<smtk::job::Queue>> g_queuesToRemove;

} // anonymous namespace

void Registrar::registerTo(const smtk::job::Manager::Ptr& jobManager)
{
  auto containerQueue = smtk::qt::job::ContainerQueue::create();
  if (containerQueue->setEngineExecutable("podman") && jobManager->queues().manage(containerQueue))
  {
    g_queuesToRemove.insert(containerQueue);
    jobManager->activeQueue().switchTo(containerQueue.get());
  }
}

void Registrar::unregisterFrom(const smtk::job::Manager::Ptr& jobManager)
{
  for (const auto& queue : g_queuesToRemove)
  {
    jobManager->queues().unmanage(queue);
  }
}

} // namespace job
} // namespace paraview
} // namespace extension
} // namespace smtk
