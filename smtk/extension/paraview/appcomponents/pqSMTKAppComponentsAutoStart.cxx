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
#include "smtk/extension/paraview/appcomponents/pqSMTKImportOperationBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"
//#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionFilterBehavior.h"
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
  auto rsrcImportOpMgr = pqSMTKImportOperationBehavior::instance(this);
  auto rsrcExportSimMgr = pqSMTKExportSimulationBehavior::instance(this);
  auto rsrcSaveMgr = pqSMTKSaveResourceBehavior::instance(this);
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk resource", rsrcMgr);
    pqCore->registerManager("smtk import operation", rsrcImportOpMgr);
    pqCore->registerManager("smtk export simulation", rsrcExportSimMgr);
    pqCore->registerManager("smtk save resource", rsrcSaveMgr);
  }
  (void)rsrcMgr;
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk resource");
    pqCore->unRegisterManager("smtk import operation");
    pqCore->unRegisterManager("smtk export simulation");
    pqCore->unRegisterManager("smtk save resource");
  }
}
