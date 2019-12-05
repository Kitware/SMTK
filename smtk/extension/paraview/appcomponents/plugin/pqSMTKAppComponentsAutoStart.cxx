//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKAppComponentsAutoStart.h"

#include "smtk/view/Selection.h"

#include "smtk/extension/paraview/appcomponents/plugin/pqPluginSMTKViewBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKCallObserversOnMainThreadBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKCloseResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKDisplayAttributeOnLoadBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKExportSimulationBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKImportIntoResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKImportOperationBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKNewResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKPipelineSelectionBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKRegisterImportersBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKSaveOnCloseResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKSaveResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/plugin/vtkSMTKEncodeSelection.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "smtk/extension/qt/qtSMTKUtilities.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"

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
    this->RegisterOverride("vtkPVEncodeSelectionForServer", "vtkSMTKEncodeSelection",
      "Override ParaView selection processing for SMTK", 1,
      []() -> vtkObject* { return vtkSMTKEncodeSelection::New(); });
  }
};
vtkStandardNewMacro(vtkSMTKAppComponentsFactory);
}

pqSMTKAppComponentsAutoStart::pqSMTKAppComponentsAutoStart(QObject* parent)
  : Superclass(parent)
{
  m_viewTracker = new pqPluginSMTKViewBehavior(parent);
}

pqSMTKAppComponentsAutoStart::~pqSMTKAppComponentsAutoStart() = default;

void pqSMTKAppComponentsAutoStart::startup()
{
  vtkNew<vtkSMTKAppComponentsFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);

  auto rsrcMgr = pqSMTKBehavior::instance(this);
  auto renderResourceBehavior = pqSMTKRenderResourceBehavior::instance(this);
  auto closeResourceBehavior = pqSMTKCloseResourceBehavior::instance(this);
  auto callObserversOnMainThread = pqSMTKCallObserversOnMainThreadBehavior::instance(this);
  auto rsrcImportOpMgr = pqSMTKImportOperationBehavior::instance(this);
  auto rsrcExportSimMgr = pqSMTKExportSimulationBehavior::instance(this);
  auto pipelineSync = pqSMTKPipelineSelectionBehavior::instance(this);
  auto displayOnLoad = pqSMTKDisplayAttributeOnLoadBehavior::instance(this);

  // The "New Resource" menu item keys off of the "Save Resource" menu item,
  // so the order of initialization for the following two global statics is
  // important!
  //
  // TODO: There must be a better way to do this.
  auto rsrcSaveMgr = pqSMTKSaveResourceBehavior::instance(this);
  auto rsrcNewMgr = pqSMTKNewResourceBehavior::instance(this);
  auto rsrcImportIntoMgr = pqSMTKImportIntoResourceBehavior::instance(this);
  auto registerImportersBehavior = pqSMTKRegisterImportersBehavior::instance(this);

  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk resource", rsrcMgr);
    pqCore->registerManager("smtk render resource", renderResourceBehavior);
    // If we are running inside CTest, don't pop up dialogs on close.
    if (!vtksys::SystemTools::GetEnv("DASHBOARD_TEST_FROM_CTEST"))
    {
      auto saveOnCloseResourceBehavior = pqSMTKSaveOnCloseResourceBehavior::instance(this);
      pqCore->registerManager("smtk save on close resource", saveOnCloseResourceBehavior);
    }
    pqCore->registerManager("call observers on main thread", callObserversOnMainThread);
    pqCore->registerManager("smtk close resource", closeResourceBehavior);
    pqCore->registerManager("smtk import operation", rsrcImportOpMgr);
    pqCore->registerManager("smtk export simulation", rsrcExportSimMgr);
    pqCore->registerManager("smtk save resource", rsrcSaveMgr);
    pqCore->registerManager("smtk new resource", rsrcNewMgr);
    pqCore->registerManager("smtk import into resource", rsrcImportIntoMgr);
    pqCore->registerManager("smtk register importers", registerImportersBehavior);
    pqCore->registerManager("smtk pipeline selection sync", pipelineSync);
    pqCore->registerManager("smtk display attribute on load", displayOnLoad);

    // If there is already an active server, create a wrapper for it.
    auto server = pqCore->getActiveServer();
    if (server)
    {
      rsrcMgr->addManagerOnServer(server);
    }
  }
  (void)rsrcMgr;

  // Dynamic creation of the resource browser
  qtSMTKUtilities::registerViewConstructor(
    "pqSMTKResourceBrowser", pqSMTKResourceBrowser::createViewWidget);
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk resource");
    pqCore->unRegisterManager("smtk render resource");
    if (!vtksys::SystemTools::GetEnv("DASHBOARD_TEST_FROM_CTEST"))
    {
      pqCore->unRegisterManager("smtk save on close resource");
    }
    pqCore->unRegisterManager("call observers on main thread");
    pqCore->unRegisterManager("smtk close resource");
    pqCore->unRegisterManager("smtk import operation");
    pqCore->unRegisterManager("smtk export simulation");
    pqCore->unRegisterManager("smtk save resource");
    pqCore->unRegisterManager("smtk new resource");
    pqCore->unRegisterManager("smtk import into resource");
    pqCore->unRegisterManager("smtk register importers");
    pqCore->unRegisterManager("smtk pipeline selection sync");
    pqCore->unRegisterManager("smtk display attribute on load");
  }
}
