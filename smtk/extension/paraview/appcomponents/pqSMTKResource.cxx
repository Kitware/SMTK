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

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKAttributeReader.h"
#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"

#include "smtk/model/Manager.h"

#include <iostream>

pqSMTKResource::pqSMTKResource(
  const QString& grp, const QString& name, vtkSMProxy* proxy, pqServer* server, QObject* parent)
  : pqPipelineSource(name, proxy, server, parent)
{
  (void)grp;
  std::cout << "SMTKResource is born! " << this << "\n";
  auto behavior = pqSMTKBehavior::instance();
  auto rsrcMgr = behavior->resourceManagerForServer(server);
  auto rsrc = this->getResource();
  if (rsrc && rsrcMgr)
  {
    rsrcMgr->smtkResourceManager()->add(rsrc);
    m_lastResource = rsrc;
  }
  QObject::connect(this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));
}

pqSMTKResource::~pqSMTKResource()
{
  QObject::disconnect(
    this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));

  auto lastRsrc = m_lastResource.lock();
  if (lastRsrc)
  {
    auto behavior = pqSMTKBehavior::instance();
    auto rsrcMgrPxy = behavior->resourceManagerForServer(this->getServer());
    if (rsrcMgrPxy)
    {
      auto rsrcMgr = rsrcMgrPxy->smtkResourceManager();
      if (rsrcMgr)
      {
        std::cout << "  Removing " << lastRsrc << " from mgr " << rsrcMgr << "\n";
        rsrcMgr->remove(lastRsrc);
      }
    }
  }
  std::cout << "Killing a resource and frowning at a small kitten\n";
}

smtk::resource::ResourcePtr pqSMTKResource::getResource() const
{
  // TODO: Actually this currently returns the server's copy and only
  //       works in built-in mode.
  smtk::resource::ResourcePtr rsrc;
  auto pxy = this->getProxy()->GetClientSideObject();
  // std::cout << "get resource from " << pxy->GetClassName() << "\n";
  auto smtkModelRdr = vtkSMTKModelReader::SafeDownCast(pxy);
  rsrc = smtkModelRdr ? smtkModelRdr->GetSMTKResource() : nullptr;
  if (rsrc)
  {
    return rsrc;
  }
  auto smtkAttributeRdr = vtkSMTKAttributeReader::SafeDownCast(pxy);
  rsrc = smtkAttributeRdr ? smtkAttributeRdr->GetSMTKResource() : nullptr;
  if (rsrc)
  {
    return rsrc;
  }
  // TODO: Handle meshes here.
  return rsrc;
}

void pqSMTKResource::dropResource()
{
  m_lastResource = smtk::resource::ResourcePtr();
}

void pqSMTKResource::synchronizeResource()
{
  // std::cout << "Re-send resource\n";
  auto lastRsrc = m_lastResource.lock();
  auto smtkRsrc = this->getResource();
  if (smtkRsrc != lastRsrc)
  {
    auto behavior = pqSMTKBehavior::instance();
    auto rsrcMgrPxy = behavior->resourceManagerForServer(this->getServer());
    auto rsrcMgr = rsrcMgrPxy ? rsrcMgrPxy->smtkResourceManager() : nullptr;
    if (!rsrcMgr)
    {
      return;
    }
    if (lastRsrc)
    {
      rsrcMgr->remove(lastRsrc);
    }
    m_lastResource = smtkRsrc;
    if (smtkRsrc)
    {
      rsrcMgr->add(smtkRsrc);
    }
    emit resourceModified(smtkRsrc);
  }
}
