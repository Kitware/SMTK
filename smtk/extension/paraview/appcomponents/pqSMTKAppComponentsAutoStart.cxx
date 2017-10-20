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

#include "smtk/resource/SelectionManager.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionFilterBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionSyncBehavior.h"

#include "pqApplicationCore.h"

pqSMTKAppComponentsAutoStart::pqSMTKAppComponentsAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKAppComponentsAutoStart::~pqSMTKAppComponentsAutoStart()
{
}

void pqSMTKAppComponentsAutoStart::startup()
{
  auto selnMgr = smtk::resource::SelectionManager::create();
  auto selnSync = new pqSMTKSelectionSyncBehavior(this, selnMgr);
  auto selnFilter = pqSMTKSelectionFilterBehavior::instance();
  if (!selnFilter)
  {
    selnFilter = new pqSMTKSelectionFilterBehavior(this, selnMgr);
  }
  else
  {
    selnFilter->setSelectionManager(selnMgr);
  }
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk selection sync", selnSync);
    pqCore->registerManager("smtk selection filter", selnFilter);
  }
}

void pqSMTKAppComponentsAutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk selection sync");
    pqCore->unRegisterManager("smtk selection filter");
  }
}
