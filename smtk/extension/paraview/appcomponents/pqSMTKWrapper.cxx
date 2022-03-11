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
#include "pqServer.h"
#include "pqServerResource.h"

#include "vtkPVSelectionSource.h"
#include "vtkSMSourceProxy.h"

#include <iostream>

#define DEBUG_WRAPPER 0

pqSMTKWrapper::pqSMTKWrapper(
  const QString& regGroup,
  const QString& regName,
  vtkSMProxy* proxy,
  pqServer* server,
  QObject* parent)
  : Superclass(regGroup, regName, proxy, server, parent)
{
#if !defined(NDEBUG) && DEBUG_WRAPPER
  std::cout << "pqResourceManager ctor " << parent << "\n";
#endif
  pqSMTKBehavior::instance()->addPQProxy(this);
}

pqSMTKWrapper::~pqSMTKWrapper()
{
#if !defined(NDEBUG) && DEBUG_WRAPPER
  std::cout << "pqResourceManager dtor\n";
#endif
  if (!this->getServer())
  {
    for (const auto& rsrc : m_resources)
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

smtk::view::ManagerPtr pqSMTKWrapper::smtkViewManager() const
{
  return this->smtkProxy() ? this->smtkProxy()->GetViewManager() : nullptr;
}

smtk::common::TypeContainer& pqSMTKWrapper::smtkManagers() const
{
  static smtk::common::TypeContainer nullContainer;
  return this->smtkProxy() ? *this->smtkProxy()->GetManagersPtr() : nullContainer;
}

smtk::common::Managers::Ptr pqSMTKWrapper::smtkManagersPtr() const
{
  smtk::common::Managers::Ptr result;
  if (auto* proxy = this->smtkProxy())
  {
    result = proxy->GetManagersPtr();
  }
  return result;
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
  for (const auto& rsrc : m_resources)
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
  auto* pxy = this->smtkProxy();
  if (pxy)
  {
    pxy->AddResourceProxy(rsrc->getSourceProxy());
  }
}

void pqSMTKWrapper::removeResource(pqSMTKResource* rsrc)
{
  m_resources.erase(rsrc);
  auto* pxy = this->smtkProxy();
  if (pxy)
  {
    pxy->RemoveResourceProxy(rsrc->getSourceProxy());
  }
}
