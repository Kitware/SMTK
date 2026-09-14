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
#include "smtk/extension/qt/qtViewRegistrar.h"

#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/TypeAndColorBadge.h"
#include "smtk/extension/qt/agents/JobRunnerAgent.h"
#include "smtk/extension/qt/diagram/qtComponentNode.h"
#include "smtk/extension/qt/diagram/qtConnectMode.h"
#include "smtk/extension/qt/diagram/qtDefaultTaskNode.h"
#include "smtk/extension/qt/diagram/qtDefaultTaskNode1.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDisconnectMode.h"
#include "smtk/extension/qt/diagram/qtPanMode.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/diagram/qtResourceNode.h"
#include "smtk/extension/qt/diagram/qtSelectMode.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/diagram/qtTaskNode.h"
#include "smtk/extension/qt/job/ShellQueue.h"
#include "smtk/extension/qt/job/UpdateContainerQueueMachine.h"
#include "smtk/extension/qt/qtAnalysisView.h"
#include "smtk/extension/qt/qtAssociationView.h"
#include "smtk/extension/qt/qtAttributeTableView.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtCategorySelectorView.h"
#include "smtk/extension/qt/qtComponentAttributeView.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtSelectorView.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"
#include "smtk/extension/qt/qtWorkletPalette.h"

#include "smtk/plugin/Manager.h"

#include "smtk/common/Paths.h"

#include "smtk/Options.h"
#include "smtk/SystemConfig.h"

#include <tuple>

#if SMTK_ENABLE_PYTHON_WRAPPING
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/operation/pybind11/PyOperation.h"
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QtDebug>

#define SMTK_DEBUG 0

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{
namespace
{
using OperationList = std::tuple<smtk::qt::job::UpdateContainerQueueMachine>;

using ViewWidgetList = std::tuple<
  qtAnalysisView,
  qtAssociationView,
  qtAttributeTableView,
  qtAttributeView,
  qtCategorySelectorView,
  qtGroupView,
  qtInstancedView,
  qtComponentAttributeView,
  qtOperationView,
  qtOperationPalette,
  qtResourceBrowser,
  qtSelectorView,
  qtSimpleExpressionView,
  qtDiagram,
  qtWorkletPalette>;

using BadgeList =
  std::tuple<smtk::extension::qt::MembershipBadge, smtk::extension::qt::TypeAndColorBadge>;

using DiagramGeneratorList = std::tuple<qtTaskEditor, qtResourceDiagram>;
using DiagramViewModeList = std::tuple<qtConnectMode, qtDisconnectMode, qtPanMode, qtSelectMode>;
using TaskNodeList = std::tuple<qtTaskNode, qtDefaultTaskNode, qtDefaultTaskNode1>;
using ObjectNodeList = std::tuple<qtResourceNode, qtComponentNode>;

using AgentList = std::tuple<smtk::task::JobRunnerAgent>;

/// A list of queues registered by this registrar (which should be removed when unregistering).
std::set<std::shared_ptr<smtk::job::Queue>> g_queuesToRemove;

} // namespace

void qtViewRegistrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(qtManager::create());
  smtk::plugin::Manager::instance()->registerPluginsTo(managers->get<qtManager::Ptr>());

#if SMTK_ENABLE_PYTHON_WRAPPING
  smtk::operation::PyOperation::runOnMainThread =
    [](smtk::operation::PyOperation::SimpleFunction fn) {
      if (!QCoreApplication::instance() || QThread::currentThread() == qApp->thread())
      {
        // We're running in the GUI thread already, just call the function:
        fn();
        return;
      }

      QMetaObject::invokeMethod(qApp, fn, Qt::BlockingQueuedConnection);
    };
#endif

  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto jobManager = managers->get<smtk::job::Manager::Ptr>();
  if (!resourceManager || !operationManager || !jobManager)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Missing managers. Cannot restore shell_queue.");
  }
  else
  {
    auto shellQueue = smtk::qt::job::ShellQueue::createOrRestore<smtk::qt::job::ShellQueue>(
      /* name */ "shell_queue",
      /* description */ R"(A queue that runs each of its jobs on the local machine.)",
      /* location */ "localhost",
      /* maximum job size */ 0,
      /* capability tags */ { "shell"_token, "bash"_token, "local"_token },
      /* remove queue on destruction */ false,
      smtk::common::UUID("e1b560df-f238-4191-80b8-40de9b63f071"),
      resourceManager,
      operationManager,
      jobManager);
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
    // Windows cannot execute a Bash script directly. Use the Bash distributed
    // with OpenFOAM and initialize its environment before running each job.
    smtk::common::Paths pp;
    auto executableDirectory = std::filesystem::path(pp.executableDirectory());
    auto openFoamRoot = executableDirectory.parent_path() / "of";
    auto msysBin = openFoamRoot / "msys64" / "usr" / "bin";
    auto interpreter = msysBin / "bash.exe";
    if (!std::filesystem::exists(interpreter))
    {
      qInfo() << "ShellQueue could not find OpenFOAM interpreter at "
              << QString::fromStdString(interpreter.string()) << ", using \"bash.exe\".";
      interpreter = "bash.exe";
    }
    shellQueue->setInterpreter(interpreter);
    // QProcess uses the native Windows environment to locate DLLs and tools
    // needed while Bash starts. The sourced OpenFOAM setup then adds its own
    // executables to the POSIX PATH seen by job scripts.
    auto environment = shellQueue->processEnvironment();
    auto path = environment.value("PATH");
    environment.insert(
      "PATH",
      QString::fromStdString(msysBin.string()) +
        (path.isEmpty() ? QString() : QDir::listSeparator() + path));
    shellQueue->setProcessEnvironment(environment);

    auto openFoamProjects = openFoamRoot / "msys64" / "home" / "ofuser" / "OpenFOAM";
    std::string openFoamProjectName;
    if (std::filesystem::is_directory(openFoamProjects))
    {
      for (const auto& entry : std::filesystem::directory_iterator(openFoamProjects))
      {
        const auto name = entry.path().filename().string();
        if (entry.is_directory() && name.rfind("OpenFOAM-", 0) == 0 && name > openFoamProjectName)
        {
          openFoamProjectName = name;
        }
      }
    }
    if (openFoamProjectName.empty())
    {
      qWarning() << "ShellQueue could not find an OpenFOAM installation under"
                 << QString::fromStdString(openFoamProjects.string());
    }
    const auto bashrc = "/home/ofuser/OpenFOAM/" + openFoamProjectName + "/etc/bashrc";

    shellQueue->setInterpreterArguments(
      { "--noprofile",
        "--norc",
        "-c",
        "export PATH=/usr/bin:$PATH; . \"" + bashrc + "\" && exec \"$1\"",
        "smtk-shell-queue" });
#endif
    g_queuesToRemove.insert(shellQueue);
    if (jobManager->queues().manage(shellQueue))
    {
      jobManager->activeQueue().switchTo(shellQueue.get());
    }
  }
}

void qtViewRegistrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<qtManager>();

#if SMTK_ENABLE_PYTHON_WRAPPING
  smtk::operation::PyOperation::runOnMainThread =
    [](smtk::operation::PyOperation::SimpleFunction fn) { fn(); };
#endif
}

void qtViewRegistrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}

void qtViewRegistrar::registerTo(const smtk::task::Manager::Ptr& taskManager)
{
  auto& agentFactory = taskManager->agentFactory();
  agentFactory.registerTypes<AgentList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::task::Manager::Ptr& taskManager)
{
  auto& agentFactory = taskManager->agentFactory();
  agentFactory.unregisterTypes<AgentList>();
}

void qtViewRegistrar::registerTo(const smtk::extension::qtManager::Ptr& qtMgr)
{
  qtMgr->diagramViewModeFactory().registerTypes<DiagramViewModeList>();
  qtMgr->diagramGeneratorFactory().registerTypes<DiagramGeneratorList>();
  qtMgr->taskNodeFactory().registerTypes<TaskNodeList>();
  qtMgr->objectNodeFactory().registerTypes<ObjectNodeList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::extension::qtManager::Ptr& qtMgr)
{
  qtMgr->diagramViewModeFactory().unregisterTypes<DiagramViewModeList>();
  qtMgr->diagramGeneratorFactory().unregisterTypes<DiagramGeneratorList>();
  qtMgr->taskNodeFactory().unregisterTypes<TaskNodeList>();
  qtMgr->objectNodeFactory().unregisterTypes<ObjectNodeList>();
}

void qtViewRegistrar::registerTo(const smtk::view::Manager::Ptr& manager)
{
  manager->viewWidgetFactory().registerTypes<ViewWidgetList>();
  // a set of user-friendly constructor names to use for alternate lookup.
  manager->viewWidgetFactory().addAlias<qtAnalysisView>("Analysis");
  manager->viewWidgetFactory().addAlias<qtAssociationView>("Associations");
  manager->viewWidgetFactory().addAlias<qtAttributeView>("Attribute");
  manager->viewWidgetFactory().addAlias<qtAttributeTableView>("AttributeTable");
  manager->viewWidgetFactory().addAlias<qtGroupView>("Group");
  manager->viewWidgetFactory().addAlias<qtInstancedView>("Instanced");
  manager->viewWidgetFactory().addAlias<qtOperationPalette>("OperationPalette");
  manager->viewWidgetFactory().addAlias<qtOperationView>("Operation");
  manager->viewWidgetFactory().addAlias<qtSelectorView>("Selector");
  manager->viewWidgetFactory().addAlias<qtSimpleExpressionView>("SimpleExpression");
  manager->viewWidgetFactory().addAlias<qtCategorySelectorView>("Category");
  // Keeping this for backward compatibility for the time being
  manager->viewWidgetFactory().addAlias<qtComponentAttributeView>("ModelEntity");
  manager->viewWidgetFactory().addAlias<qtComponentAttributeView>("ComponentAttribute");
  manager->viewWidgetFactory().addAlias<qtResourceBrowser>("ResourceBrowser");
  manager->viewWidgetFactory().addAlias<qtDiagram>("Diagram");
  manager->viewWidgetFactory().addAlias<qtWorkletPalette>("WorkletPalette");

  manager->badgeFactory().registerTypes<BadgeList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::view::Manager::Ptr& manager)
{
  manager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();

  manager->badgeFactory().unregisterTypes<BadgeList>();
}

void qtViewRegistrar::registerTo(const smtk::job::Manager::Ptr& jobManager) {}

void qtViewRegistrar::unregisterFrom(const smtk::job::Manager::Ptr& jobManager)
{
  for (const auto& queue : g_queuesToRemove)
  {
    jobManager->queues().unmanage(queue);
  }
}

} // namespace extension
} // namespace smtk
