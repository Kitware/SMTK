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

#include "smtk/extension/qt/RedirectOutput.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/operators/ImportPythonOperation.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqView.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSessionIterator.h"

#include <QCoreApplication>
#include <QObject>

#define DEBUG_PQSMTKBEHAVIOR 0

class pqSMTKBehavior::Internal
{
public:
  std::map<pqServer*, std::pair<vtkSMSMTKWrapperProxy*, pqSMTKWrapper*>> Remotes;
  std::map<
    std::weak_ptr<smtk::resource::Resource>,
    QPointer<pqSMTKResource>,
    std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
    ResourceMap;
};

static pqSMTKBehavior* g_instance = nullptr;

pqSMTKBehavior::pqSMTKBehavior(QObject* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  // Blech: pqApplicationCore doesn't have the selection manager yet,
  // so wait until we hear that the server is ready to make the connection.
  // We can't have a selection before the first connection, anyway.
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    auto* builder = pqCore->getObjectBuilder();
    QObject::connect(
      builder, SIGNAL(proxyCreated(pqProxy*)), this, SLOT(handleNewSMTKProxies(pqProxy*)));
    QObject::connect(
      builder,
      SIGNAL(destroying(pqPipelineSource*)),
      this,
      SLOT(handleOldSMTKProxies(pqPipelineSource*)));

    QObject::connect(
      pqCore->getServerManagerModel(),
      SIGNAL(serverReady(pqServer*)),
      this,
      SLOT(addManagerOnServer(pqServer*)));
    QObject::connect(
      pqCore->getServerManagerModel(),
      SIGNAL(aboutToRemoveServer(pqServer*)),
      this,
      SLOT(removeManagerFromServer(pqServer*)));

    // TODO: Do we need to call serverReady manually if pqActiveObjects says there's already an active server?
  }

  // Redirect the singleton smtk::io::Logger::instance() to Qt's I/O stream,
  // which in turn is picked up by ParaView's output widget.
  smtk::extension::qt::RedirectOutputToQt(this, smtk::io::Logger::instance());
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

vtkSMSMTKWrapperProxy* pqSMTKBehavior::wrapperProxy(pqServer* remote) const
{
  auto entry = remote ? m_p->Remotes.find(remote) : m_p->Remotes.begin();
  if (entry == m_p->Remotes.end())
  {
    return nullptr;
  }
  return entry->second.first;
}

pqSMTKWrapper* pqSMTKBehavior::resourceManagerForServer(pqServer* remote) const
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

  auto* server = rsrcMgr->getServer();
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

  Q_EMIT addedManagerOnServer(rsrcMgr, server);
}

pqSMTKWrapper* pqSMTKBehavior::getPVResourceManager(smtk::resource::ManagerPtr mgr)
{
  pqSMTKWrapper* result = nullptr;
  this->visitResourceManagersOnServers([&result, &mgr](pqSMTKWrapper* mos, pqServer* /*unused*/) {
    if (mos && mos->smtkResourceManager() == mgr)
    {
      result = mos;
      return true;
    }
    return false;
  });
  return result;
}

QPointer<pqSMTKResource> pqSMTKBehavior::getPVResource(
  const smtk::resource::ResourcePtr& resource) const
{
  QPointer<pqSMTKResource> result = nullptr;
  auto it = m_p->ResourceMap.find(resource);
  if (it != m_p->ResourceMap.end())
  {
    result = it->second;
  }
  return result;
}

vtkSMProxy* pqSMTKBehavior::getPVResourceProxy(const smtk::resource::ResourcePtr& rsrc) const
{
  auto pvSource = this->getPVResource(rsrc);
  if (!pvSource)
  {
    pqSMTKBehavior::processEvents();
    pvSource = this->getPVResource(rsrc);
  }
  if (pvSource)
  {
    return pvSource->getProxy();
  }
  return nullptr;
}

void pqSMTKBehavior::processEvents()
{
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
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
  auto* app = pqApplicationCore::instance();
  if (!app)
  {
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
    std::cout << "No PV application for SMTK!\n";
#endif
    return;
  }

  if (server->isRemote())
  {
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
    std::cout << "PV server is remote\n";
#endif
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
  auto* rmpxy = dynamic_cast<vtkSMSMTKWrapperProxy*>(pxy);
  m_p->Remotes[server].first = rmpxy;

  // Increase the shared pointer count to prevent the wrapper from being
  // deleted until we explicitly delete it.
  rmpxy->GetSession()->Register(nullptr);

  Q_EMIT addedManagerOnServer(rmpxy, server);
}

void pqSMTKBehavior::removeManagerFromServer(pqServer* remote)
{
#if !defined(NDEBUG) && DEBUG_PQSMTKRESOURCE
  std::cout << "Removing rsrc mgr from server: " << remote << "\n\n";
#endif
  auto entry = m_p->Remotes.find(remote);
  if (entry == m_p->Remotes.end())
  {
    return;
  }

  // Clear the client-side resource manager
  if (entry->second.first && entry->second.first->GetResourceManager())
  {
    entry->second.first->GetResourceManager()->clear();
  }

  // Notify listeners that the client-side managers are going away
  Q_EMIT removingManagerFromServer(entry->second.first, entry->first);

  // Notify listeners that the server-side managers are going away
  Q_EMIT removingManagerFromServer(entry->second.second, entry->first);

  // Decrease the shared pointer count to allow the wrapper to go out of scope.
  entry->second.first->GetSession()->UnRegister(nullptr);

  m_p->Remotes.erase(entry);
}

void pqSMTKBehavior::handleNewSMTKProxies(pqProxy* pxy)
{
  auto* rsrcPxy = dynamic_cast<pqSMTKResource*>(pxy);
  if (rsrcPxy)
  {
    auto rsrc = rsrcPxy->getResource();
    if (rsrc)
    {
      m_p->ResourceMap[rsrc] = rsrcPxy;
    }
    // Monitor rsrcPxy so that if its assigned SMTK resource changes, we update it.
    QObject::connect(
      rsrcPxy, &pqSMTKResource::resourceModified, this, &pqSMTKBehavior::updateResourceProxyMap);
    auto it = m_p->Remotes.find(rsrcPxy->getServer());
    if (it == m_p->Remotes.end())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Behavior didn't have resource manager for proxy's server.");
    }
    if (it != m_p->Remotes.end() && it->second.second)
    {
      it->second.second->addResource(rsrcPxy);
    }
  }
}

void pqSMTKBehavior::handleOldSMTKProxies(pqPipelineSource* pxy)
{
  auto* rsrcPxy = dynamic_cast<pqSMTKResource*>(pxy);
  if (rsrcPxy)
  {
    m_p->ResourceMap.erase(rsrcPxy->getResource());
    auto it = m_p->Remotes.find(rsrcPxy->getServer());
    if (it != m_p->Remotes.end())
    {
      it->second.second->removeResource(rsrcPxy);
    }
    return;
  }
}

void pqSMTKBehavior::updateResourceProxyMap(
  const std::shared_ptr<smtk::resource::Resource>& resource)
{
  auto* rsrcPxy = dynamic_cast<pqSMTKResource*>(this->sender());
  if (rsrcPxy && resource)
  {
    m_p->ResourceMap[resource] = rsrcPxy;
  }
}

bool pqSMTKBehavior::createRepresentation(pqSMTKResource* pvr, pqView* view)
{
  auto* source = qobject_cast<pqPipelineSource*>(pvr);
  auto* pqPort = source ? source->getOutputPort(0) : nullptr;
  if (!pqPort || !view)
    return false;

  auto* pqCore = pqApplicationCore::instance();
  if (!pqCore)
    return false;

  auto* builder = pqCore->getObjectBuilder();
  pqDataRepresentation* pqRep = builder->createDataRepresentation(pqPort, view);
  if (pqRep)
  {
    this->setDefaultRepresentationVisibility(pqPort, view);
    pqRep->renderViewEventually();
    return true;
  }
  return false;
}

pqSMTKWrapper* pqSMTKBehavior::builtinOrActiveWrapper() const
{
  pqSMTKWrapper* builtin = nullptr;
  pqSMTKWrapper* existing = nullptr;
  this->visitResourceManagersOnServers(
    [&builtin, &existing](pqSMTKWrapper* wrapper, pqServer* server) -> bool {
      if (wrapper && !existing)
      {
        existing = wrapper;
      }
      if (server->getResource().scheme() == "builtin")
      {
        builtin = wrapper;
        return true;
      }
      return false;
    });

  if (!builtin)
  {
    auto* app = pqApplicationCore::instance();
    if (!app)
    {
      return existing;
    }
    pqServer* server = app->getActiveServer();
    if (server)
    {
      builtin = this->resourceManagerForServer(server);
    }
    if (!builtin)
    {
      builtin = existing;
    }
  }
  return builtin;
}

void pqSMTKBehavior::updateWrapperMap()
{
  auto* pqCore = pqApplicationCore::instance();
  if (!pqCore)
  {
    return;
  }
  auto* smm = pqCore->getServerManagerModel();
  if (!smm)
  {
    return;
  }
  auto* pm = vtkProcessModule::GetProcessModule();
  if (!pm)
  {
    return;
  }

  auto* it = pm->NewSessionIterator();
  for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
  {
    auto* session = it->GetCurrentSession();
    if (!session)
    {
      continue;
    }
    auto* server = smm->findServer(session);
    if (!server)
    {
      continue;
    }
    auto remoteIt = m_p->Remotes.find(server);
    if (remoteIt != m_p->Remotes.end())
    {
      continue;
    } // We already have an entry.

    // Point the server to one of the wrappers (only one should exist,
    // but we loop over them all and the last one wins):
    auto wrappers = smm->findItems<pqSMTKWrapper*>();
    for (const auto& wrapper : wrappers)
    {
      if (wrapper && wrapper->getProxy())
      {
        m_p->Remotes[server] = std::pair<vtkSMSMTKWrapperProxy*, pqSMTKWrapper*>(
          vtkSMSMTKWrapperProxy::SafeDownCast(wrapper->getProxy()), wrapper);
      }
    }
  }
  it->Delete();
}

void pqSMTKBehavior::importPythonOperationsForModule(
  const std::string& moduleName,
  const std::string& operationName)
{
#ifdef SMTK_PYTHON_ENABLED
  QTimer::singleShot(0, [moduleName, operationName]() {
    auto* behavior = pqSMTKBehavior::instance();
    auto* wrapper = behavior ? behavior->builtinOrActiveWrapper() : nullptr;
    auto opMgr = wrapper ? wrapper->smtkOperationManager() : nullptr;
    if (opMgr)
    {
      smtk::operation::ImportPythonOperation::importOperation(*opMgr, moduleName, operationName);
    }
  });
#else
  static bool once = false;
  if (!once)
  {
    once = true;
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Python is not enabled; no operation will be imported.");
  }
#endif
}

bool pqSMTKBehavior::setPostProcessingMode(bool inPost)
{
  if (inPost == m_postProcessingMode)
  {
    return false;
  }
  m_postProcessingMode = inPost;
  Q_EMIT postProcessingModeChanged(m_postProcessingMode);
  return true;
}

void pqSMTKBehavior::setDefaultRepresentationVisibility(pqOutputPort* pqPort, pqView* view)
{
  // ControllerWithRendering finds and uses the appropriate vtkSMRepresentationProxy
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  controller->Show(pqPort->getSourceProxy(), pqPort->getPortNumber(), view->getViewProxy());
}
