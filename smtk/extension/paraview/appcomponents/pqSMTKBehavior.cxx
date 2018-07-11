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
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"

#include <QObject>

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

namespace
{
std::string trim(const std::string& str)
{
  std::size_t first = str.find_first_not_of(' ');
  if (first == std::string::npos)
  {
    return str;
  }
  std::size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

void extensionsAndDescriptionsFromFileFilters(const std::string& fileFilters,
  std::vector<std::pair<std::string, std::string> >& extensionsAndDescriptions)
{
  regex re(";;");
  sregex_token_iterator it(fileFilters.begin(), fileFilters.end(), re, -1), last;
  for (; it != last; ++it)
  {
    std::size_t begin = it->str().find_first_not_of(" \n\r\t*.", it->str().find_last_of("("));
    std::size_t end = it->str().find_last_not_of(" \n\r\t", it->str().find_last_of(")"));
    std::string description = it->str().substr(0, begin);
    std::string extensions = it->str().substr(begin + 1, end - begin - 1);

    extensions = regex_replace(extensions, regex("[ ]{0,}\\*."), " ");
    extensions = regex_replace(extensions, regex("[ ]{0,}\\*"), "");
    if (extensions.empty())
    {
      continue;
    }

    extensionsAndDescriptions.push_back(std::make_pair(trim(extensions), trim(description)));
  }
}

std::string xmlForSMTKImporter(
  const std::string& resource, const std::string& extensions, const std::string& description)
{
  static int i = 0;
  std::stringstream s;
  s << "<ServerManagerConfiguration>\n";
  s << "  <ProxyGroup name=\"sources\">\n";
  s << "    <SourceProxy name=\"SMTKModelImporter_" << i++ << " \" class=\"vtkSMTKModelImporter\" ";
  s << "label=\"SMTK importer for " << description << " into " << resource << "\">\n";
  s << "      <Documentation>\n";
  s << "        short_help=\"Import a " << description << " as an SMTK " << resource << ".\"\n";
  s << "      </Documentation>\n";

  s << R"(
      <StringVectorProperty
        name="FileName"
        command="SetFileName"
        animateable="0"
        number_of_elements="1">
        <FileListDomain name="files"/>
        <Documentation>
          The path of a file to read.
        </Documentation>
      </StringVectorProperty>)";
  s << "\n";
  s << R"(
      <StringVectorProperty
        name="ResourceName"
        command="SetResourceName"
        number_of_elements="1")";
  s << "\n        default_values=\"" <<  resource << "\"\n";
  s << R"(
       panel_visibility="never">
        <Documentation>
          The resource type into which the file is imported.
        </Documentation>
      </StringVectorProperty>
      <OutputPort index="0" name="model entities"/>
      <OutputPort index="1" name="instance prototypes"/>
      <OutputPort index="2" name="instance points"/>
)";

  s << "      <Hints>\n";
  s << "        <ReaderFactory extensions=\"" << extensions << "\" ";
  s << "file_description=\"" << description << "\"/>";
  s << R"(
        <Representation view="RenderView" type="SMTKModelCompositeRepresentation" />
      </Hints>
    </SourceProxy>
  </ProxyGroup>
</ServerManagerConfiguration>
)";
  return s.str();
}

void registerSMTKImporter(
  pqServer* server, const std::string& resource, const std::string& fileFilters)
{
  auto app = pqApplicationCore::instance();
  if (app)
  {
    std::vector<std::pair<std::string, std::string> > extensionsAndDescriptions;
    extensionsAndDescriptionsFromFileFilters(fileFilters, extensionsAndDescriptions);
    for (auto& token : extensionsAndDescriptions)
    {
      server->proxyManager()->GetProxyDefinitionManager()->LoadConfigurationXMLFromString(
        xmlForSMTKImporter(resource, token.first, token.second).c_str());
    }
  }
}
}

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

  // Redirect the singleton smtk::io::Logger::instance() to Qt's I/O stream,
  // which in turn is picked up by ParaView's output widget.
  smtk::extension::qt::RedirectOutputToQt(smtk::io::Logger::instance());
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

  auto importerGroup = smtk::operation::ImporterGroup(rsrcMgr->smtkOperationManager());
  for (auto& resourceName : importerGroup.supportedResources())
  {
    for (auto index : importerGroup.operationsForResource(resourceName))
    {
      auto fileItemDef = importerGroup.fileItemDefinitionForOperation(index);
      registerSMTKImporter(server, resourceName, fileItemDef->getFileFilters());
    }
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
