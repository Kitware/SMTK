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
#include "smtk/job/Queue.h"

#include <set>

using namespace smtk::string::literals;

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

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  const char* cep = smtkSettings->GetContainerEnginePath();
  std::string containerEnginePath = cep && cep[0] ? cep : "podman";

  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto jobManager = managers->get<smtk::job::Manager::Ptr>();
  if (!resourceManager || !operationManager || !jobManager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Missing managers. Cannot restore container_queue.");
  }
  else
  {
    auto containerQueue =
      smtk::qt::job::ContainerQueue::createOrRestore<smtk::qt::job::ContainerQueue>(
        /* name */ "container_queue",
        /* description */
        R"(A queue that runs each of its jobs in a container hosted by the local machine.)",
        /* location */ "localhost",
        /* maximum job size */ 0,
        /* capability tags */ { "container"_token, "bash"_token, "local"_token },
        containerEnginePath,
        /* remove queue on destruction */ false,
        smtk::common::UUID("d865c244-55c1-4b37-8d91-e179196db51a"),
        resourceManager,
        operationManager,
        jobManager);
    g_queuesToRemove.insert(containerQueue);
    if (jobManager->queues().manage(containerQueue))
    {
      jobManager->activeQueue().switchTo(containerQueue.get());
    }
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  auto jobManager = managers->get<smtk::job::Manager::Ptr>();
  if (jobManager)
  {
    for (const auto& queue : g_queuesToRemove)
    {
      jobManager->queues().unmanage(queue);
    }
  }
}

void Registrar::registerTo(const smtk::job::Manager::Ptr& jobManager)
{
  (void)jobManager;
}

void Registrar::unregisterFrom(const smtk::job::Manager::Ptr& jobManager) {}

} // namespace job
} // namespace paraview
} // namespace extension
} // namespace smtk
