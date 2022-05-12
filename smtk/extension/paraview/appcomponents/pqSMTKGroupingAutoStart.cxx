//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKGroupingAutoStart.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKGroupComponentsBehavior.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPVApplicationCore.h"
#include "pqTestUtility.h"

#include "vtkObjectFactory.h"
#include "vtkVersion.h"
#include "vtksys/SystemTools.hxx"

pqSMTKGroupingAutoStart::pqSMTKGroupingAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKGroupingAutoStart::~pqSMTKGroupingAutoStart() = default;

void pqSMTKGroupingAutoStart::startup()
{
  auto* groupComponentsBehavior = pqSMTKGroupComponentsBehavior::instance(this);

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk group components", groupComponentsBehavior);
  }
}

void pqSMTKGroupingAutoStart::shutdown()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk group components");
  }
}
