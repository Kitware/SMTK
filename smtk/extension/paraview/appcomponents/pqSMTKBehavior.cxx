//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "smtk/io/Logger.h"

// Client side
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqView.h"
#include "vtkNew.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMSourceProxy.h"

#include <QObject>

class pqSMTKBehavior::Internal
{
public:
  std::map<pqServer*, std::pair<vtkSMSMTKWrapperProxy*, pqSMTKWrapper*> > Remotes;
};

static pqSMTKBehavior* g_instance = nullptr;

pqSMTKBehavior::pqSMTKBehavior(QObject* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  // Blech: pqApplicationCore doesn't have the selection manager yet,
  // so wait until we hear that the server is ready to make the connection.
  // We can't have a selection before the first connection, anyway.
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    auto builder = pqCore->getObjectBuilder();
    QObject::connect(
      builder, SIGNAL(proxyCreated(pqProxy*)), this, SLOT(handleNewSMTKProxies(pqProxy*)));
    QObject::connect(
      builder, SIGNAL(destroying(pqProxy*)), this, SLOT(handleOldSMTKProxies(pqProxy*)));

    QObject::connect(pqCore->getServerManagerModel(), SIGNAL(serverReady(pqServer*)), this,
      SLOT(addManagerOnServer(pqServer*)));
    QObject::connect(pqCore->getServerManagerModel(), SIGNAL(aboutToRemoveServer(pqServer*)), this,
      SLOT(removeManagerFromServer(pqServer*)));
    // TODO: Do we need to call serverReady manually if pqActiveObjects says there's already an active server?
  }
}

pqSMTKBehavior* pqSMTKBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKBehavior::~pqSMTKBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
  while (!m_p->Remotes.empty())
  {
    removeManagerFromServer(m_p->Remotes.begin()->first);
  }
  delete m_p;
}

vtkSMSMTKWrapperProxy* pqSMTKBehavior::wrapperProxy(pqServer* remote)
{
  auto entry = remote ? m_p->Remotes.find(remote) : m_p->Remotes.begin();
  if (entry == m_p->Remotes.end())
  {
    return nullptr;
  }
  return entry->second.first;
}

pqSMTKWrapper* pqSMTKBehavior::resourceManagerForServer(pqServer* remote)
{
  auto entry = remote ? m_p->Remotes.find(remote) : m_p->Remotes.begin();
  if (entry == m_p->Remotes.end())
  {
    return nullptr;
  }
  return entry->second.second;
}

void pqSMTKBehavior::addPQProxy(pqSMTKWrapper* rsrcMgr)
{
  if (!rsrcMgr)
  {
    return;
  }

  auto server = rsrcMgr->getServer();
  auto it = m_p->Remotes.find(server);
  if (it == m_p->Remotes.end())
  {
    m_p->Remotes[server] = std::pair<vtkSMSMTKWrapperProxy*, pqSMTKWrapper*>(
      vtkSMSMTKWrapperProxy::SafeDownCast(rsrcMgr->getProxy()), rsrcMgr);
  }
  else
  {
    it->second.second = rsrcMgr;
  }
  emit addedManagerOnServer(rsrcMgr, server);
}

pqSMTKWrapper* pqSMTKBehavior::getPVResourceManager(smtk::resource::ManagerPtr mgr)
{
  pqSMTKWrapper* result = nullptr;
  this->visitResourceManagersOnServers([&result, &mgr](pqSMTKWrapper* mos, pqServer*) {
    if (mos && mos->smtkResourceManager() == mgr)
    {
      result = mos;
      return true;
    }
    return false;
  });
  return result;
}

pqSMTKResource* pqSMTKBehavior::getPVResource(smtk::resource::ResourcePtr mgr)
{
  pqSMTKResource* result = nullptr;
  this->visitResourceManagersOnServers([&result, &mgr](pqSMTKWrapper* mos, pqServer*) {
    pqSMTKResource* pvr;
    if (mos && (pvr = mos->getPVResource(mgr)))
    {
      result = pvr;
      return true;
    }
    return false;
  });
  return result;
}

void pqSMTKBehavior::visitResourceManagersOnServers(
  const std::function<bool(pqSMTKWrapper*, pqServer*)>& fn) const
{
  for (auto remote : m_p->Remotes)
  {
    if (fn(remote.second.second, remote.first))
    {
      break;
    }
  }
}

void pqSMTKBehavior::addManagerOnServer(pqServer* server)
{
  auto app = pqApplicationCore::instance();
  if (!app)
  {
    std::cout << "No PV application for SMTK!\n";
    return;
  }
  pqObjectBuilder* builder = app->getObjectBuilder();

  // Set up the resource manager.

  // TODO: Monitor app->getServerManagerModel()'s serverReady/serverRemoved events
  //       and add/remove resource managers as required.

  // This creates a vtkSMSMTKWrapperProxy on the client and a
  // vtkSMTKWrapper on the server. Because our plugin uses
  // the add_pqproxy() cmake macro, this also results in the creation of
  // a pqSMTKWrapper instance so that Qt events (notably selection
  // changes) can trigger SMTK events.
  vtkSMProxy* pxy = builder->createProxy("smtk", "SMTKWrapper", server, "smtk resources");
  auto rmpxy = dynamic_cast<vtkSMSMTKWrapperProxy*>(pxy);
  m_p->Remotes[server].first = rmpxy;

  emit addedManagerOnServer(rmpxy, server);
}

void pqSMTKBehavior::removeManagerFromServer(pqServer* remote)
{
  std::cout << "Removing rsrc mgr from server: " << remote << "\n\n";
  auto entry = m_p->Remotes.find(remote);
  if (entry == m_p->Remotes.end())
  {
    return;
  }
  emit removingManagerFromServer(entry->second.first, entry->first);
  emit removingManagerFromServer(entry->second.second, entry->first);
  m_p->Remotes.erase(entry);
}

void pqSMTKBehavior::handleNewSMTKProxies(pqProxy* pxy)
{
  auto rsrc = dynamic_cast<pqSMTKResource*>(pxy);
  if (rsrc)
  {
    auto it = m_p->Remotes.find(rsrc->getServer());
    if (it == m_p->Remotes.end())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Behavior didn't have resource manager for proxy's server.");
    }
    if (it != m_p->Remotes.end() && it->second.second)
    {
      it->second.second->addResource(rsrc);
    }
    return;
  }
}

void pqSMTKBehavior::handleOldSMTKProxies(pqProxy* pxy)
{
  auto rsrc = dynamic_cast<pqSMTKResource*>(pxy);
  if (rsrc)
  {
    auto it = m_p->Remotes.find(rsrc->getServer());
    if (it != m_p->Remotes.end())
    {
      it->second.second->removeResource(rsrc);
    }
    return;
  }
}

bool pqSMTKBehavior::createRepresentation(pqSMTKResource* pvr, pqView* view)
{
  auto source = qobject_cast<pqPipelineSource*>(pvr);
  auto pqPort = source ? source->getOutputPort(0) : nullptr;
  if (!pqPort || !view)
    return false;

  auto pqCore = pqApplicationCore::instance();
  if (!pqCore)
    return false;

  auto builder = pqCore->getObjectBuilder();
  pqDataRepresentation* pqRep = builder->createDataRepresentation(pqPort, view);
  if (pqRep)
  {
    this->setDefaultRepresentationVisibility(pqPort, view);
    pqRep->renderViewEventually();
    return true;
  }
  return false;
}

void pqSMTKBehavior::setDefaultRepresentationVisibility(pqOutputPort* pqPort, pqView* view)
{
  // ControllerWithRendering finds and uses the appropriate vtkSMRepresentationProxy
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  controller->Show(pqPort->getSourceProxy(), pqPort->getPortNumber(), view->getViewProxy());
}
