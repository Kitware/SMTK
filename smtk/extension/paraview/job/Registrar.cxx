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

#include "smtk/extension/paraview/job/ArtifactStageTracker.h"
#include "smtk/extension/paraview/job/pqArtifacts.h"
#include "smtk/extension/paraview/job/pqJobRunnerView.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/qt/job/ContainerQueue.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Queue.h"

#include "pqApplicationCore.h"
#include "pqPropertyLinks.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "vtkSMProxy.h"
#include "vtkSMSessionProxyManager.h"

#include <QPointer>
#include <QTimer>
#include <QtGlobal> // For Q_OS_WIN

#include <map>
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
pqPropertyLinks g_containerEngineLink;
QPointer<pqServerManagerModel> g_settingsModel;

// Observe completion independently of the lifetime of any task's view.
std::map<smtk::operation::Manager*, smtk::operation::Observers::Key> g_artifactObservers;

// This is called whenever a new pqServer attaches and monitors the SMTK settings for
// changes. Copy the loaded container engine into each queue before linking its
// project root, since changing the root can launch the container engine.
// If the "ProjectsRootFolder" property is modified, any queues created by
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
  g_containerEngineLink.removeAllPropertyLinks();
  for (const auto& queue : g_queuesToRemove)
  {
    // This one-way link initializes queues created before settings were loaded
    // and tracks applied settings changes. Do not copy queue defaults back into
    // the user's settings.
    g_containerEngineLink.addPropertyLink(
      queue.get(),
      "engineExecutable",
      "",
      smtkProxy,
      smtkProxy->GetProperty("ContainerEnginePath"));
    g_projectRootLink.addPropertyLink(
      queue.get(),
      "rootJobDirectory",
      SIGNAL(rootJobDirectoryChanging(const std::filesystem::path&, const std::filesystem::path&)),
      smtkProxy,
      smtkProxy->GetProperty("ProjectsRootFolder"));
  }
}

// When a server disconnects, disconnect any property links.
void serverDisconnect(pqServer*)
{
  g_projectRootLink.removeAllPropertyLinks();
  g_containerEngineLink.removeAllPropertyLinks();
}

// Bind queues to settings once the application and its server are ready.
// Registration may happen before saved settings have been loaded.
void syncSettings()
{
  auto* core = pqApplicationCore::instance();
  if (!core)
  {
    QTimer::singleShot(50 /*ms*/, &syncSettings);
    return;
  }
  if (g_settingsModel != core->getServerManagerModel())
  {
    g_settingsModel = core->getServerManagerModel();
    QObject::connect(g_settingsModel.data(), &pqServerManagerModel::serverReady, &serverConnect);
    QObject::connect(
      g_settingsModel.data(), &pqServerManagerModel::aboutToRemoveServer, &serverDisconnect);
  }
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
    if (g_artifactObservers.find(operationManager.get()) == g_artifactObservers.end())
    {
      // The observer owns tracking for this manager, independently of task views.
      // Observe all modified-job results, not just JobUpdated: ScheduleJob also
      // modifies jobs and must reset tracking when a UUID is reused for another run.
      auto tracker = std::make_shared<detail::ArtifactStageTracker>();
      g_artifactObservers.emplace(
        operationManager.get(),
        operationManager->observers().insert(
          [tracker](
            const smtk::operation::Operation&,
            smtk::operation::EventType event,
            smtk::operation::Operation::Result result) {
            if (event == smtk::operation::EventType::DID_OPERATE)
            {
              for (const auto& component : *result->findComponent("modified"))
              {
                auto job = std::dynamic_pointer_cast<smtk::job::Job>(component);
                if (job)
                {
                  const auto stages = tracker->update(*job);
                  if (stages.first < stages.second)
                  {
                    pqArtifacts::instance()->reload(*job, stages.first, stages.second);
                  }
                }
              }
              for (const auto& component : *result->findComponent("expunged"))
              {
                if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(component))
                {
                  tracker->erase(job->id());
                }
              }
            }
            return 0;
          },
          0,
          false,
          "Reload artifacts as job stages complete."));
    }
#ifndef Q_OS_WIN
    auto containerQueue =
      smtk::qt::job::ContainerQueue::createOrRestore<smtk::qt::job::ContainerQueue>(
        /* name */ "container_queue",
        /* description */
        R"(A queue that runs each of its jobs in a container hosted by the local machine.)",
        /* location */ "localhost",
        /* maximum job size */ -1,
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
      // Make the container queue the active queue active, except
      // on windows. (We should undo this change when possible.)
      jobManager->activeQueue().switchTo(containerQueue.get());
    }
    syncSettings();
#endif // Q_OS_WIN
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  if (auto operationManager = managers->get<smtk::operation::Manager::Ptr>())
  {
    g_artifactObservers.erase(operationManager.get());
  }
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
