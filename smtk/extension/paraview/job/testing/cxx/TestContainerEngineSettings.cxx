//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/**\brief Verify that container queues honor the applied container engine setting.
 *
 * The regression allowed Preferences to display a saved absolute Podman path
 * while the C++ settings object and job queue retained the default "podman"
 * command. On macOS, jobs could then work when launched from Terminal but fail
 * when launched from Finder, whose PATH did not include Podman's directory.
 *
 * This test loads the real SMTK settings plugin and exercises the job registrar
 * with a built-in ParaView session. It checks that:
 * - A queue created before settings load receives the saved absolute path.
 * - The settings proxy forwards the path to the C++ settings object.
 * - Pending edits leave queues unchanged; applied edits update the path and
 *   engine type in existing queues.
 * - Queues created after settings load receive the configured path as well.
 * - Clearing the setting restores "podman" in every queue without writing that
 *   fallback back into the user's empty setting.
 *
 * Paths containing spaces are intentional. No container engine is executed:
 * the project root remains empty, and the executable paths need not exist.
 * Queue storage and Qt preferences use temporary directories. CTest passes the
 * settings plugin path as argv[1] and --disable-registry to avoid loading or
 * saving the user's ParaView settings. This tests settings propagation, not
 * Finder launching, executable discovery, VM startup, or container execution.
 */
#include "smtk/common/Managers.h"
#include "smtk/common/Paths.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/extension/paraview/job/Registrar.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/qt/job/ContainerQueue.h"
#include "smtk/job/Manager.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "vtkSMPluginManager.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSettings.h"
#include "vtkSMUncheckedPropertyHelper.h"

#include <QApplication>
#include <QSettings>
#include <QTemporaryDir>

namespace
{
// Isolate the real registrar's SQLite queue persistence on every platform,
// including macOS, where changing HOME does not change Application Support.
class TestPaths : public smtk::common::Paths
{
public:
  static void useConfigurationDirectory(const std::string& directory)
  {
    TestPaths paths;
    paths.userConfigurationDirectory();
    s_userConfigurationDirectory = directory;
  }
};
} // namespace

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  QTemporaryDir directory;
  test(directory.isValid(), "Create isolated queue storage.");
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
  pqApplicationCore core(argc, argv);
  test(
    vtkSMProxyManager::GetProxyManager()->GetPluginManager()->LoadLocalPlugin(argv[1]),
    "Load the real SMTK settings proxy and its client-server bindings.");
  TestPaths::useConfigurationDirectory(directory.path().toStdString());

  auto resources = smtk::resource::Manager::create();
  auto operations = smtk::operation::Manager::create();
  auto jobs = smtk::job::Manager::create();
  auto managers = smtk::common::Managers::create();
  managers->insert(resources);
  managers->insert(operations);
  managers->insert(jobs);
  // Create the queue before the session loads its saved settings. An empty
  // project root avoids starting or reconfiguring any Podman machine.
  smtk::extension::paraview::job::Registrar::registerTo(managers);
  auto* queue = dynamic_cast<smtk::qt::job::ContainerQueue*>(jobs->defaultQueue());
  test(queue != nullptr, "The registrar must create an active container queue.");
  test(queue->engineExecutable() == "podman", "The initial queue uses the default engine.");

  vtkSMSettings::GetInstance()->ClearAllSettings();
  vtkSMSettings::GetInstance()->AddCollectionFromString(
    R"({"settings":{"SMTKSettings":{"ContainerEnginePath":"/test tools/podman"}}})", 1.0);
  auto* server = core.getObjectBuilder()->createServer(pqServerResource("builtin:"));
  test(server != nullptr, "Create a session that loads saved settings.");
  auto* settings = server->proxyManager()->GetProxy("settings", "SMTKSettings");
  test(settings != nullptr, "The session must register SMTK settings.");
  test(
    queue->engineExecutable() == "/test tools/podman",
    "A queue created before settings load must receive the saved absolute path.");
  test(
    std::string(vtkSMTKSettings::GetInstance()->GetContainerEnginePath()) == "/test tools/podman",
    "The settings proxy must call the C++ container engine setter.");

  vtkSMUncheckedPropertyHelper(settings, "ContainerEnginePath").Set("/other tools/docker");
  test(
    queue->engineExecutable() == "/test tools/podman",
    "Unapplied settings edits must not change the engine used by existing jobs.");
  vtkSMPropertyHelper(settings, "ContainerEnginePath").Set("/other tools/docker");
  settings->UpdateVTKObjects();
  test(
    queue->engineExecutable() == "/other tools/docker" && queue->engine().data() == "docker",
    "Changing the applied setting must update the existing queue's path and engine.");
  test(
    std::string(vtkSMTKSettings::GetInstance()->GetContainerEnginePath()) == "/other tools/docker",
    "Applied changes must also reach the C++ settings object.");

  auto lateManagers = smtk::common::Managers::create();
  auto lateJobs = smtk::job::Manager::create();
  lateManagers->insert(smtk::resource::Manager::create());
  lateManagers->insert(smtk::operation::Manager::create());
  lateManagers->insert(lateJobs);
  smtk::extension::paraview::job::Registrar::registerTo(lateManagers);
  auto* lateQueue = dynamic_cast<smtk::qt::job::ContainerQueue*>(lateJobs->defaultQueue());
  test(
    lateQueue && lateQueue->engineExecutable() == "/other tools/docker",
    "Queues registered after settings load must also receive the configured engine.");

  vtkSMPropertyHelper(settings, "ContainerEnginePath").Set("");
  settings->UpdateVTKObjects();
  test(
    queue->engineExecutable() == "podman" && lateQueue->engineExecutable() == "podman",
    "Clearing the path must restore the default engine in all registered queues.");
  test(
    std::string(vtkSMPropertyHelper(settings, "ContainerEnginePath").GetAsString()).empty(),
    "The queue's default must not overwrite the user's empty setting.");

  smtk::extension::paraview::job::Registrar::unregisterFrom(lateManagers);
  smtk::extension::paraview::job::Registrar::unregisterFrom(managers);
  core.getObjectBuilder()->removeServer(server);
  return 0;
}
