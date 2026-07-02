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

#include "smtk/extension/paraview/job/pqJobRunnerView.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/qt/job/ContainerQueue.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/job/Manager.h"
#include "smtk/job/Queue.h"

#include "pqApplicationCore.h"
#include "pqPropertyLinks.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "vtkSMProxy.h"
#include "vtkSMSessionProxyManager.h"

#include <QTimer>

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
std::set<std::shared_ptr<smtk::qt::job::ContainerQueue>> g_queuesToRemove;

pqPropertyLinks g_projectRootLink;

// This is called whenever a new pqServer attaches and monitors the SMTK settings for
// changes. If the "ProjectsRootFolder" property is modified, any queues created by
// this registrar have their "rootJobDirectory" updated to match so that the
// host OS maps the "ProjectsRootFolder" to its VM (allowing individual containers to
// access each job's case directory).
void serverConnect(pqServer* server)
{
  vtkSMProxy* smtkProxy = server->proxyManager()->GetProxy("settings", "SMTKSettings");
  if (!smtkProxy)
  {
    return;
  }
  g_projectRootLink.removeAllPropertyLinks();
  for (const auto& queue : g_queuesToRemove)
  {
    g_projectRootLink.addPropertyLink(
      queue.get(),
      "rootJobDirectory",
      SIGNAL(rootJobDirectoryChanging(const std::filesystem::path&, const std::filesystem::path&)),
      smtkProxy,
      smtkProxy->GetProperty("ProjectsRootFolder"));
  }
}

// When a server disconnects, disconnect any property links.
void serverDisconnect(pqServer* server)
{
  g_projectRootLink.removeAllPropertyLinks();
}

// This is called to reset the container queue's virtual machine
// (WSL2/HyperV on Windows, ??? on MacOS) as the "ProjectsRootFolder"
// is modified by users in the settings dialog.
void syncSettingsProjectsRootFolder()
{
  auto* core = pqApplicationCore::instance();
  if (!core)
  {
    QTimer::singleShot(50 /*ms*/, &syncSettingsProjectsRootFolder);
    return;
  }
  QObject::connect(
    core->getServerManagerModel(), &pqServerManagerModel::serverReady, &serverConnect);
  QObject::connect(
    core->getServerManagerModel(), &pqServerManagerModel::aboutToRemoveServer, &serverDisconnect);
  if (auto* server = core->getActiveServer())
  {
    serverConnect(server);
  }
}

} // anonymous namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  const char* cep = smtkSettings->GetContainerEnginePath();
  std::string containerEnginePath = cep && cep[0] ? cep : "podman";
  const char* prf = smtkSettings->GetProjectsRootFolder();
  std::string projectsRootFolder = (prf && prf[0] ? prf : "");

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
        projectsRootFolder,
        /* docker UID */ -1,
        /* docker GID */ -1,
        resourceManager,
        operationManager,
        jobManager);
    g_queuesToRemove.insert(containerQueue);
    g_projectRootLink.setAutoUpdateVTKObjects(true);
    if (jobManager->queues().manage(containerQueue))
    {
      jobManager->activeQueue().switchTo(containerQueue.get());
    }
    syncSettingsProjectsRootFolder();
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

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerType<pqJobRunnerView>();
  viewManager->viewWidgetFactory().addAlias<pqJobRunnerView>("JobRunner");
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterType<pqJobRunnerView>();
  viewManager->viewWidgetFactory().unregisterType("JobRunner");
}

void Registrar::registerTo(const smtk::job::Manager::Ptr& jobManager)
{
  (void)jobManager;
}

void Registrar::unregisterFrom(const smtk::job::Manager::Ptr& jobManager)
{
  (void)jobManager;
}

} // namespace job
} // namespace paraview
} // namespace extension
} // namespace smtk
