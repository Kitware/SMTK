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
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"
#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"

#include "smtk/io/Logger.h"

// Client side
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "vtkSMSourceProxy.h"

#include <QObject>

class pqSMTKBehavior::Internal
{
public:
  std::map<pqServer*, std::pair<vtkSMSMTKResourceManagerProxy*, pqSMTKResourceManager*> > Remotes;
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

vtkSMSMTKResourceManagerProxy* pqSMTKBehavior::wrapperProxy(pqServer* remote)
{
  auto entry = remote ? m_p->Remotes.find(remote) : m_p->Remotes.begin();
  if (entry == m_p->Remotes.end())
  {
    return nullptr;
  }
  return entry->second.first;
}

pqSMTKResourceManager* pqSMTKBehavior::resourceManagerForServer(pqServer* remote)
{
  auto entry = remote ? m_p->Remotes.find(remote) : m_p->Remotes.begin();
  if (entry == m_p->Remotes.end())
  {
    return nullptr;
  }
  return entry->second.second;
}

void pqSMTKBehavior::addPQProxy(pqSMTKResourceManager* rsrcMgr)
{
  if (!rsrcMgr)
  {
    return;
  }

  auto server = rsrcMgr->getServer();
  auto it = m_p->Remotes.find(server);
  if (it == m_p->Remotes.end())
  {
    m_p->Remotes[server] = std::pair<vtkSMSMTKResourceManagerProxy*, pqSMTKResourceManager*>(
      vtkSMSMTKResourceManagerProxy::SafeDownCast(rsrcMgr->getProxy()), rsrcMgr);
  }
  else
  {
    it->second.second = rsrcMgr;
  }
  emit addedManagerOnServer(rsrcMgr, server);
}

void pqSMTKBehavior::visitResourceManagersOnServers(
  const std::function<void(pqSMTKResourceManager*, pqServer*)>& fn) const
{
  for (auto remote : m_p->Remotes)
  {
    fn(remote.second.second, remote.first);
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

  // This creates a vtkSMSMTKResourceManagerProxy on the client and a
  // vtkSMTKResourceManagerWrapper on the server. Because our plugin uses
  // the add_pqproxy() cmake macro, this also results in the creation of
  // a pqSMTKResourceManager instance so that Qt events (notably selection
  // changes) can trigger SMTK events.
  vtkSMProxy* pxy = builder->createProxy("smtk", "ResourceManager", server, "smtk resources");
  auto rmpxy = dynamic_cast<vtkSMSMTKResourceManagerProxy*>(pxy);
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
