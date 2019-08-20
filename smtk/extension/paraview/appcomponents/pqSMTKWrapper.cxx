//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceSource.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/operation/Manager.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"

// PV Client side
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqSelectionManager.h"

#include "vtkSMSourceProxy.h"

#include <iostream>

#define DEBUG_WRAPPER 0

pqSMTKWrapper::pqSMTKWrapper(const QString& regGroup, const QString& regName, vtkSMProxy* proxy,
  pqServer* server, QObject* parent)
  : Superclass(regGroup, regName, proxy, server, parent)
{
#if !defined(NDEBUG) && DEBUG_WRAPPER
  std::cout << "pqResourceManager ctor " << parent << "\n";
#endif
  // I. Listen for PV selections and convert them to SMTK selections
  bool listening = false;
  auto app = pqApplicationCore::instance();
  if (app)
  {
    auto pvSelnMgr = qobject_cast<pqSelectionManager*>(app->manager("SelectionManager"));
    if (pvSelnMgr)
    {
      if (QObject::connect(pvSelnMgr, SIGNAL(selectionChanged(pqOutputPort*)), this,
            SLOT(paraviewSelectionChanged(pqOutputPort*))))
      {
        listening = true;
      }
    }
  }
  if (!listening)
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Could not connect SMTK resource manager to ParaView selection manager.");
  }

  // II. Listen for operation events and signal them.
  //     Note that what we **should** be doing is listening for these
  //     events on a client-side operation manager used to forward
  //     operations to the server. What we in fact do only works for
  //     the built-in mode. TODO: Fix this. Remove the need for me.
  auto pxy = vtkSMSMTKWrapperProxy::SafeDownCast(this->getProxy());
  auto wrapper = vtkSMTKWrapper::SafeDownCast(pxy->GetClientSideObject());
  if (wrapper)
  {
    /*
    wrapper->GetManager()->observe([this](
        smtk::resource::Event event,
        const smtk::resource::ResourcePtr& rsrc)
      {
        switch (event)
        {
        case smtk::resource::Event::RESOURCE_ADDED: emit resourceAdded(rsrc); break;
        case smtk::resource::Event::RESOURCE_REMOVED: emit resourceRemoved(rsrc); break;
        }
      }
    );
    */
    wrapper->GetOperationManager()->observers().insert(
      [this](const smtk::operation::Operation& oper, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        emit operationEvent(oper, event, result);
        return 0;
      });
  }
  pqSMTKBehavior::instance()->addPQProxy(this);
}

pqSMTKWrapper::~pqSMTKWrapper()
{
#if !defined(NDEBUG) && DEBUG_WRAPPER
  std::cout << "pqResourceManager dtor\n";
#endif
  if (!this->getServer())
  {
    for (auto rsrc : m_resources)
    {
      if (rsrc)
      {
#if !defined(NDEBUG) && DEBUG_WRAPPER
        std::cout << "  finalize " << rsrc << "\n";
#endif
        rsrc->dropResource();
      }
    }
  }
}

vtkSMSMTKWrapperProxy* pqSMTKWrapper::smtkProxy() const
{
  return vtkSMSMTKWrapperProxy::SafeDownCast(this->getProxy());
}

smtk::resource::ManagerPtr pqSMTKWrapper::smtkResourceManager() const
{
  return this->smtkProxy() ? this->smtkProxy()->GetResourceManager() : nullptr;
}

smtk::operation::ManagerPtr pqSMTKWrapper::smtkOperationManager() const
{
  return this->smtkProxy() ? this->smtkProxy()->GetOperationManager() : nullptr;
}

smtk::view::SelectionPtr pqSMTKWrapper::smtkSelection() const
{
  return this->smtkProxy() ? this->smtkProxy()->GetSelection() : nullptr;
}

smtk::project::ManagerPtr pqSMTKWrapper::smtkProjectManager() const
{
  return this->smtkProxy() ? this->smtkProxy()->GetProjectManager() : nullptr;
}

pqSMTKResource* pqSMTKWrapper::getPVResource(const smtk::resource::ResourcePtr& rsrc) const
{
  pqSMTKResource* result = nullptr;
  this->visitResources([&result, &rsrc](pqSMTKResource* pvrsrc) {
    if (pvrsrc && pvrsrc->getResource() == rsrc)
    {
      result = pvrsrc;
      return true;
    }
    return false;
  });
  return result;
}

void pqSMTKWrapper::visitResources(std::function<bool(pqSMTKResource*)> visitor) const
{
  for (auto rsrc : m_resources)
  {
    if (rsrc)
    {
      if (visitor(rsrc))
      {
        break;
      }
    }
  }
}

void pqSMTKWrapper::addResource(pqSMTKResource* rsrc)
{
  m_resources.insert(rsrc);
  auto pxy = this->smtkProxy();
  if (pxy)
  {
    pxy->AddResourceProxy(rsrc->getSourceProxy());
    emit resourceAdded(rsrc);
  }
}

void pqSMTKWrapper::removeResource(pqSMTKResource* rsrc)
{
  m_resources.erase(rsrc);
  auto pxy = this->smtkProxy();
  if (pxy)
  {
    emit resourceRemoved(rsrc);
    pxy->RemoveResourceProxy(rsrc->getSourceProxy());
  }
}

void pqSMTKWrapper::paraviewSelectionChanged(pqOutputPort* port)
{
  // When we support client/server separation, this is where client-side
  // selection logic will go. Server-side selection logic is currently handled
  // in vtkSMTKSelectionRepresentation.
  //
  // With the builtin server, the same smtk::view::Selection object is held by
  // the wrapper proxy object and the server-side wrapper object. This will be
  // different for remote servers and require synchronization in this method.
  (void)port;
}
