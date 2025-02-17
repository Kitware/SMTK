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
#include "smtk/extension/paraview/server/vtkSMTKResource.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceSource.h"

#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "vtkSMSourceProxy.h"

#define DEBUG_PQSMTKRESOURCE 0

#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
#include <iostream>
#endif

using namespace smtk::string::literals;

pqSMTKResource::pqSMTKResource(
  const QString& grp,
  const QString& name,
  vtkSMProxy* proxy,
  pqServer* server,
  QObject* parent)
  : pqPipelineSource(name, proxy, server, parent)
{
  (void)grp;
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
  std::cout << "Creating SMTKResource ( " << this << " )\n";
#endif
  auto* behavior = pqSMTKBehavior::instance();
  auto* rsrcMgr = behavior->resourceManagerForServer(server);
  auto rsrc = this->getResource();
  if (rsrc && rsrcMgr)
  {
    rsrcMgr->smtkResourceManager()->add(rsrc);
    m_lastResource = rsrc;
  }
  QObject::connect(this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));

  // Define an observer that adds all created resources to the resource manager.
  // This observer is called by smtk::operation::Operation::operate(), which may
  // be run in a subthread. We wish to use this signal to update the render
  // window, a task which must be executed on the main program thread. We
  // therefore use this observer to emit a signal that is connected to a lambda
  // that has this class instance as its context.
  m_key = rsrcMgr->smtkOperationManager()->observers().insert(
    [&](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result result) {
      if (event == smtk::operation::EventType::DID_OPERATE)
      {
        auto affectedResources = smtk::operation::extractResources(result);
        auto targetResource = this->getResource();
        if (affectedResources.find(targetResource) == affectedResources.end() || !targetResource)
        {
          return 0; // Target resource is not being affected
        }
        // Only provide geometry updates on Signal operations for attribute resources.
        if (
          targetResource->matchesType("smtk::attribute::Resource"_token) ||
          !dynamic_cast<const smtk::attribute::Signal*>(&op))
        {
          Q_EMIT this->operationOccurred(QPrivateSignal());
        }
      }
      return 0;
    },
    "pqSMTKResource: Emit a Qt signal upon operation completion.");

  QObject::connect(this, &pqSMTKResource::operationOccurred, this, [&, proxy]() {
    this->getSourceProxy()->MarkDirty(proxy);
    this->setModifiedState(pqProxy::MODIFIED);
    proxy->MarkAllPropertiesAsModified();
    vtkObject::SafeDownCast(proxy->GetClientSideObject())->Modified();
    proxy->UpdateVTKObjects();
    this->renderAllViews();
  });
}

pqSMTKResource::~pqSMTKResource()
{
  /* FIXME:
   * This causes a segfault on exit because the pqSMTKWrapper has been
   * deleted out from under us. However, if the resource is deleted
   * _before_ exit, then not removing the observer can cause a segfault
   * later when an operation is performed.
  auto behavior = pqSMTKBehavior::instance();
  auto rsrcMgr = behavior->resourceManagerForServer(this->getServer());
  rsrcMgr->smtkOperationManager()->observers().erase(m_key);
   */

  QObject::disconnect(
    this, SIGNAL(dataUpdated(pqPipelineSource*)), this, SLOT(synchronizeResource()));

  auto lastRsrc = m_lastResource.lock();
  if (lastRsrc)
  {
    auto* behavior = pqSMTKBehavior::instance();
    auto* rsrcMgrPxy = behavior->resourceManagerForServer(this->getServer());
    if (rsrcMgrPxy)
    {
      auto rsrcMgr = rsrcMgrPxy->smtkResourceManager();
      if (rsrcMgr)
      {
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
        std::cout << "  Removing " << lastRsrc << " from mgr " << rsrcMgr << "\n";
#endif
        rsrcMgr->remove(lastRsrc);
      }

      auto opMgr = rsrcMgrPxy->smtkOperationManager();
      if (opMgr)
      {
        opMgr->observers().erase(m_key);
      }
    }
  }
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
  std::cout << "Destroying SMTKResource " << this << " )\n";
#endif
}

smtk::resource::ResourcePtr pqSMTKResource::getResource() const
{
  // TODO: Actually this currently returns the server's copy and only
  //       works in built-in mode.
  smtk::resource::ResourcePtr rsrc;
  auto* pxy = this->getProxy()->GetClientSideObject();
  auto* smtkRsrcRdr = vtkSMTKResourceSource::SafeDownCast(pxy);
  rsrc = smtkRsrcRdr ? smtkRsrcRdr->GetVTKResource()->GetResource() : nullptr;
  if (rsrc)
  {
    return rsrc;
  }
  auto* smtkRsrc = vtkSMTKResource::SafeDownCast(pxy);
  rsrc = smtkRsrc ? smtkRsrc->GetResource() : nullptr;
  if (rsrc)
  {
    return rsrc;
  }

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
    auto* behavior = pqSMTKBehavior::instance();
    auto* rsrcMgrPxy = behavior->resourceManagerForServer(this->getServer());
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
    Q_EMIT resourceModified(smtkRsrc);
  }
}
