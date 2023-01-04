//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKGuiAppComponentsAutoStart.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKCloseResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKImportIntoResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKNewResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"
#ifdef SMTK_PYTHON_ENABLED
#include "smtk/extension/paraview/appcomponents/pqSMTKExportSimulationBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKImportOperationBehavior.h"
#endif

#include <pqApplicationCore.h>

pqSMTKGuiAppComponentsAutoStart::pqSMTKGuiAppComponentsAutoStart(QObject* parent)
  : QObject(parent)
{
  // Does nothing
}

void pqSMTKGuiAppComponentsAutoStart::startup()
{
  auto* closeResourceBehavior = pqSMTKCloseResourceBehavior::instance(this);
#ifdef SMTK_PYTHON_ENABLED
  auto* rsrcImportOpMgr = pqSMTKImportOperationBehavior::instance(this);
  auto* rsrcExportSimMgr = pqSMTKExportSimulationBehavior::instance(this);
#endif

  // The "New Resource" menu item keys off of the "Save Resource" menu item,
  // so the order of initialization for the following two global statics is
  // important!
  //
  // TODO: There must be a better way to do this.
  auto* rsrcSaveMgr = pqSMTKSaveResourceBehavior::instance(this);
  auto* rsrcNewMgr = pqSMTKNewResourceBehavior::instance(this);
  auto* rsrcImportIntoMgr = pqSMTKImportIntoResourceBehavior::instance(this);

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk close resource", closeResourceBehavior);
#ifdef SMTK_PYTHON_ENABLED
    pqCore->registerManager("smtk import operation", rsrcImportOpMgr);
    pqCore->registerManager("smtk export simulation", rsrcExportSimMgr);
#endif
    pqCore->registerManager("smtk save resource", rsrcSaveMgr);
    pqCore->registerManager("smtk new resource", rsrcNewMgr);
    pqCore->registerManager("smtk import into resource", rsrcImportIntoMgr);
  }
}

void pqSMTKGuiAppComponentsAutoStart::shutdown()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk close resource");
#ifdef SMTK_PYTHON_ENABLED
    pqCore->unRegisterManager("smtk import operation");
    pqCore->unRegisterManager("smtk export simulation");
#endif
    pqCore->unRegisterManager("smtk save resource");
    pqCore->unRegisterManager("smtk new resource");
    pqCore->unRegisterManager("smtk import into resource");
  }
}
