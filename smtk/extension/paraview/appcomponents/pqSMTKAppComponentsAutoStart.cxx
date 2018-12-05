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

#include "smtk/extension/paraview/appcomponents/pqPluginSMTKViewBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKExportSimulationBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKImportIntoResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKImportOperationBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKNewResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKPipelineSelectionBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRegisterImportersBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"

vtkSMProxy* pqSMTKAppComponentsAutoStart::s_resourceManager = nullptr;

pqSMTKAppComponentsAutoStart::pqSMTKAppComponentsAutoStart(QObject* parent)
  : Superclass(parent)
{
  m_viewTracker = new pqPluginSMTKViewBehavior(parent);
}

pqSMTKAppComponentsAutoStart::~pqSMTKAppComponentsAutoStart()
{
}

void pqSMTKAppComponentsAutoStart::startup()
{
  auto rsrcMgr = pqSMTKBehavior::instance(this);
  auto renderResourceBehavior = pqSMTKRenderResourceBehavior::instance(this);
  auto rsrcImportOpMgr = pqSMTKImportOperationBehavior::instance(this);
  auto rsrcExportSimMgr = pqSMTKExportSimulationBehavior::instance(this);
  auto pipelineSync = pqSMTKPipelineSelectionBehavior::instance(this);

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
    pqCore->registerManager("smtk import operation", rsrcImportOpMgr);
    pqCore->registerManager("smtk export simulation", rsrcExportSimMgr);
    pqCore->registerManager("smtk save resource", rsrcSaveMgr);
    pqCore->registerManager("smtk new resource", rsrcNewMgr);
    pqCore->registerManager("smtk import into resource", rsrcImportIntoMgr);
    pqCore->registerManager("smtk register importers", registerImportersBehavior);
    pqCore->registerManager("smtk pipeline selection sync", pipelineSync);
  }
  (void)rsrcMgr;
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk resource");
    pqCore->unRegisterManager("smtk render resource");
    pqCore->unRegisterManager("smtk import operation");
    pqCore->unRegisterManager("smtk export simulation");
    pqCore->unRegisterManager("smtk save resource");
    pqCore->unRegisterManager("smtk new resource");
    pqCore->unRegisterManager("smtk import into resource");
    pqCore->unRegisterManager("smtk register importers");
    pqCore->unRegisterManager("smtk pipeline selection sync");
  }
}
