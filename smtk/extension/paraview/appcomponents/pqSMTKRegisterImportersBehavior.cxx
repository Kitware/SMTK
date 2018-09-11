//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKRegisterImportersBehavior.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "smtk/operation/groups/ImporterGroup.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqServerManagerModel.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMSessionProxyManager.h"

#include <functional>

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
  std::size_t hash = std::hash<std::string>{}(resource + description);
  std::stringstream s;
  s << "<ServerManagerConfiguration>\n";
  s << "  <ProxyGroup name=\"sources\">\n";
  s << "    <SourceProxy name=\"SMTKModelImporter_" << hash << " \" class=\"vtkSMTKSource\" ";
  s << "label=\"SMTK importer for " << description << " into " << resource << "\">\n";
  s << "      <Documentation>\n";
  s << "        short_help=\"Import a " << description << " as an SMTK " << resource << ".\"\n";
  s << "      </Documentation>\n";

  s << R"(
      <SubProxy command="SetVTKResource">
        <Proxy name="Resource"
               proxygroup="smtk_internal_sources"
               proxyname="SMTKModelImporter">
        </Proxy>
        <ExposedProperties>
          <Property name="FileName" />)";
  s << "\n          <Property name=\"ResourceName\" default_values=\"" << resource << "\" "
    << "panel_visibility=\"never\"/>\n";
  s << R"(
        </ExposedProperties>
      </SubProxy>

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

static pqSMTKRegisterImportersBehavior* g_instance = nullptr;

pqSMTKRegisterImportersBehavior::pqSMTKRegisterImportersBehavior(QObject* parent)
  : Superclass(parent)
{
  QObject::connect(pqSMTKBehavior::instance(),
    (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) & pqSMTKBehavior::addedManagerOnServer,
    this, &pqSMTKRegisterImportersBehavior::constructModelImporters);
}

pqSMTKRegisterImportersBehavior* pqSMTKRegisterImportersBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKRegisterImportersBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKRegisterImportersBehavior::~pqSMTKRegisterImportersBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}

void pqSMTKRegisterImportersBehavior::constructModelImporters(
  pqSMTKWrapper* rsrcMgr, pqServer* server)
{
  if (!rsrcMgr)
  {
    return;
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
}
