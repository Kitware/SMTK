//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"

#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"

#include <iostream>

pqSMTKResource::pqSMTKResource(
  const QString& grp, const QString& name, vtkSMProxy* proxy, pqServer* server, QObject* parent)
  : pqPipelineSource(name, proxy, server, parent)
{
  (void)grp;
  std::cout << "SMTKResource is born!\n";
  auto mgr = vtkSMSMTKResourceManagerProxy::Instance();
  if (mgr)
  {
    mgr->AddResourceProxy(this->getSourceProxy());
  }
  QObject::connect(this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));
}

pqSMTKResource::~pqSMTKResource()
{
  auto mgr = vtkSMSMTKResourceManagerProxy::Instance();
  if (mgr)
  {
    mgr->RemoveResourceProxy(this->getSourceProxy());
  }
  QObject::disconnect(
    this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));
  std::cout << "Killing a resource and frowning at a small kitten\n";
}

void pqSMTKResource::synchronizeResource()
{
  std::cout << "Re-send resource\n";
  //vtkSMSMTKResourceManagerProxy::Instance()->
}
