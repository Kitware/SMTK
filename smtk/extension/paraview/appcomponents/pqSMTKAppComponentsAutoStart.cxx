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
//#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionFilterBehavior.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"

vtkSMProxy* pqSMTKAppComponentsAutoStart::s_resourceManager = nullptr;

pqSMTKAppComponentsAutoStart::pqSMTKAppComponentsAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKAppComponentsAutoStart::~pqSMTKAppComponentsAutoStart()
{
}

void pqSMTKAppComponentsAutoStart::startup()
{
  auto rsrcMgr = pqSMTKBehavior::instance(this);
  std::cout << "Startup seln mgr behavior" << rsrcMgr << "\n";
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk resource", rsrcMgr);
  }
  (void)rsrcMgr;
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk resource");
  }
}
