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

#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
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
  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  const char* cep = smtkSettings->GetContainerEnginePath();
  std::string containerEnginePath = cep && cep[0] ? cep : "podman";

  auto containerQueue = smtk::qt::job::ContainerQueue::create();
  containerQueue->setName("container_queue");
  containerQueue->setDescription(R"(A queue that runs each of its jobs inside a container.)");
  containerQueue->setEngineExecutable(containerEnginePath);
  std::unordered_set<smtk::string::Token> tags{ "container", "docker", "shell", "bash", "local" };
  for (const auto& tag : tags)
  {
    containerQueue->addTag(tag);
  }
  if (jobManager->queues().manage(containerQueue))
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
