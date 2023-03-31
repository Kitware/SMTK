//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKAppComponentsAutoStart.h"

#include "smtk/view/Selection.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKCallObserversOnMainThreadBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKCloseWithActiveOperationBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKDisplayAttributeOnLoadBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationHintsBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKPipelineSelectionBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRegisterImportersBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveOnCloseResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/appcomponents/pqToolboxEventPlayer.h"
#include "smtk/extension/paraview/appcomponents/pqToolboxEventTranslator.h"
#include "smtk/extension/paraview/appcomponents/vtkSMTKEncodeSelection.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#ifdef SMTK_PYTHON_ENABLED
#include "smtk/extension/paraview/appcomponents/pqSMTKPythonTrace.h"
#endif

#include "smtk/extension/qt/qtSMTKUtilities.h"

#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqObjectBuilder.h"
#include "pqPVApplicationCore.h"
#include "pqTestUtility.h"

#include "vtkObjectFactory.h"
#include "vtkVersion.h"
#include "vtksys/SystemTools.hxx"

namespace
{
class vtkSMTKAppComponentsFactory : public vtkObjectFactory
{
public:
  static vtkSMTKAppComponentsFactory* New();
  vtkTypeMacro(vtkSMTKAppComponentsFactory, vtkObjectFactory);
  const char* GetDescription() override { return "SMTK app-components overrides."; }
  const char* GetVTKSourceVersion() override { return VTK_SOURCE_VERSION; }
  vtkSMTKAppComponentsFactory()
  {
    this->RegisterOverride(
      "vtkPVEncodeSelectionForServer",
      "vtkSMTKEncodeSelection",
      "Override ParaView selection processing for SMTK",
      1,
      []() -> vtkObject* { return vtkSMTKEncodeSelection::New(); });
  }
};
vtkStandardNewMacro(vtkSMTKAppComponentsFactory);
} // namespace

class pqSMTKAppComponentsAutoStart::pqInternal
{
public:
  smtk::operation::Observers::Key m_opObserver;
#ifdef SMTK_PYTHON_ENABLED
  pqSMTKPythonTrace m_pythonTrace;
#endif
};

pqSMTKAppComponentsAutoStart::pqSMTKAppComponentsAutoStart(QObject* parent)
  : Superclass(parent)
{
  m_p = new pqInternal();
}

pqSMTKAppComponentsAutoStart::~pqSMTKAppComponentsAutoStart()
{
  delete m_p;
}

void pqSMTKAppComponentsAutoStart::startup()
{
  // Set Qt's message pattern to simply print the message. SMTK's logger will
  // include the severity and file/line if requested.
  qSetMessagePattern("%{message}");

  vtkNew<vtkSMTKAppComponentsFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);

  auto* behavior = pqSMTKBehavior::instance(this);
  auto* renderResourceBehavior = pqSMTKRenderResourceBehavior::instance(this);
  auto* callObserversOnMainThread = pqSMTKCallObserversOnMainThreadBehavior::instance(this);
  auto* pipelineSync = pqSMTKPipelineSelectionBehavior::instance(this);
  auto* displayOnLoad = pqSMTKDisplayAttributeOnLoadBehavior::instance(this);
  auto* registerImportersBehavior = pqSMTKRegisterImportersBehavior::instance(this);

  auto* mainWindow = pqCoreUtilities::mainWidget();
  auto* operationHints = pqSMTKOperationHintsBehavior::instance(mainWindow);

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk resource", behavior);
    pqCore->registerManager("smtk render resource", renderResourceBehavior);
    // If we are running inside CTest, don't pop up dialogs on close.
    if (!vtksys::SystemTools::GetEnv("DASHBOARD_TEST_FROM_CTEST"))
    {
      auto* saveOnCloseResourceBehavior = pqSMTKSaveOnCloseResourceBehavior::instance(this);
      pqCore->registerManager("smtk save on close resource", saveOnCloseResourceBehavior);

      auto* closeWithActiveOperationBehavior =
        pqSMTKCloseWithActiveOperationBehavior::instance(this);
      pqCore->registerManager("smtk close with active operation", closeWithActiveOperationBehavior);
    }
    pqCore->registerManager("call observers on main thread", callObserversOnMainThread);
    pqCore->registerManager("smtk register importers", registerImportersBehavior);
    pqCore->registerManager("smtk pipeline selection sync", pipelineSync);
    pqCore->registerManager("smtk display attribute on load", displayOnLoad);
    pqCore->registerManager("operation hints", operationHints);

    QObject::connect(
      behavior,
      SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
      this,
      SLOT(observeWrapper(pqSMTKWrapper*, pqServer*)));
    QObject::connect(
      behavior,
      SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
      this,
      SLOT(unobserveWrapper(pqSMTKWrapper*, pqServer*)));
    // If there is already an active server, create a wrapper for it.
    auto* server = pqCore->getActiveServer();
    if (server)
    {
      behavior->addManagerOnServer(server);
    }
  }

  // Connect pipeline source created/destroyed signals to pqSMTKBehavior
  QObject::connect(
    renderResourceBehavior,
    &pqSMTKRenderResourceBehavior::pipelineSourceCreated,
    behavior,
    &pqSMTKBehavior::pipelineSourceCreated);
  QObject::connect(
    renderResourceBehavior,
    &pqSMTKRenderResourceBehavior::aboutToDestroyPipelineSource,
    behavior,
    &pqSMTKBehavior::aboutToDestroyPipelineSource);

  // Add an event translator for the toolbox panel (which will not
  // be present unless its plugin is loaded).
  pqPVApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqToolboxEventTranslator(this));
  pqPVApplicationCore::instance()->testUtility()->eventPlayer()->addWidgetEventPlayer(
    new pqToolboxEventPlayer(this));
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk resource");
    pqCore->unRegisterManager("smtk render resource");
    if (!vtksys::SystemTools::GetEnv("DASHBOARD_TEST_FROM_CTEST"))
    {
      pqCore->unRegisterManager("smtk save on close resource");
      pqCore->unRegisterManager("smtk close with active operation");
    }
    pqCore->unRegisterManager("call observers on main thread");
    pqCore->unRegisterManager("smtk register importers");
    pqCore->unRegisterManager("smtk pipeline selection sync");
  }
}

void pqSMTKAppComponentsAutoStart::observeWrapper(pqSMTKWrapper* wrapper, pqServer* /*server*/)
{
  m_p->m_opObserver = wrapper->smtkOperationManager()->observers().insert(
    [this](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result const &
      /*result*/) -> int {
      (void)op;
      if (event == smtk::operation::EventType::WILL_OPERATE)
      {
      // Trace operation in python
#ifdef SMTK_PYTHON_ENABLED
        this->m_p->m_pythonTrace.traceOperation(op);
#endif
      }
      return 0;
    },
    std::numeric_limits<smtk::operation::Observers::Priority>::lowest(),
    /* invoke observer on current selection */ false,
    "pqSMTKAppComponentsAutoStart: observe operations and trace in python.");
}

void pqSMTKAppComponentsAutoStart::unobserveWrapper(
  pqSMTKWrapper* /*wrapper*/,
  pqServer* /*server*/)
{
  m_p->m_opObserver = smtk::operation::Observers::Key();
}
