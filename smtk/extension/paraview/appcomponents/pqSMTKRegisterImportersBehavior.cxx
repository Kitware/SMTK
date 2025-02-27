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

#include "smtk/Regex.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqServerManagerModel.h"
#include "vtkPVXMLParser.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMSessionProxyManager.h"

#include <functional>

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

void extensionsAndDescriptionsFromFileFilters(
  const std::string& fileFilters,
  std::vector<std::pair<std::string, std::string>>& extensionsAndDescriptions)
{
  smtk::regex re(";;");
  smtk::sregex_token_iterator it(fileFilters.begin(), fileFilters.end(), re, -1), last;
  for (; it != last; ++it)
  {
    std::size_t begin = it->str().find_first_not_of(" \n\r\t*.", it->str().find_last_of('('));
    std::size_t end = it->str().find_last_not_of(" \n\r\t", it->str().find_last_of(')'));
    std::string description = it->str().substr(0, begin);
    std::string extensions = it->str().substr(begin + 1, end - begin - 1);

    extensions = smtk::regex_replace(extensions, smtk::regex("[ ]{0,}\\*."), " ");
    extensions = smtk::regex_replace(extensions, smtk::regex("[ ]{0,}\\*"), "");
    if (extensions.empty())
    {
      continue;
    }

    extensionsAndDescriptions.emplace_back(trim(extensions), trim(description));
  }
}

std::string proxyName(
  const std::string& resource,
  const std::string& /*unused*/,
  const std::string& description)
{
  std::size_t hash = std::hash<std::string>{}(resource + description);
  std::stringstream s;
  s << "SMTKResourceImporter_" << hash;
  return s.str();
}

std::string xmlForSMTKImporter(
  const std::string& resource,
  const std::string& extensions,
  const std::string& description)
{
  std::stringstream s;
  s << "<ServerManagerConfiguration>\n";
  s << "  <ProxyGroup name=\"sources\">\n";
  s << "    <SourceProxy name=\"" << proxyName(resource, extensions, description)
    << "\" class=\"vtkSMTKResourceSource\" ";
  s << "label=\"SMTK importer for " << description << " into " << resource << "\">\n";
  s << "      <Documentation>\n";
  s << "        short_help=\"Import a " << description << " as an SMTK " << resource << ".\"\n";
  s << "      </Documentation>\n";

  s << R"(
      <SubProxy command="SetVTKResource">
        <Proxy name="Resource"
               proxygroup="smtk_internal_sources"
               proxyname="SMTKResourceImporter">
        </Proxy>
        <ExposedProperties>
          <Property name="FileName" />)";
  s << "\n          <Property name=\"ResourceName\" default_values=\"" << resource << "\" "
    << "panel_visibility=\"never\"/>\n";
  s << R"(
        </ExposedProperties>
      </SubProxy>

      <OutputPort index="0" name="components"/>
      <OutputPort index="1" name="instance prototypes"/>
      <OutputPort index="2" name="instance points"/>
)";

  s << "      <Hints>\n";
  s << "        <ReaderFactory extensions=\"" << extensions << "\" ";
  s << "file_description=\"" << description << "\"/>";
  s << R"(
        <Representation view="RenderView" type="SMTKResourceCompositeRepresentation" />
      </Hints>
    </SourceProxy>
  </ProxyGroup>
</ServerManagerConfiguration>
)";
  return s.str();
}

void registerSMTKImporter(
  pqServer* server,
  const std::string& resource,
  const std::string& fileFilters)
{
  auto* app = pqApplicationCore::instance();
  if (app)
  {
    std::vector<std::pair<std::string, std::string>> extensionsAndDescriptions;
    extensionsAndDescriptionsFromFileFilters(fileFilters, extensionsAndDescriptions);
    for (auto& token : extensionsAndDescriptions)
    {
      vtkNew<vtkPVXMLParser> parser;
      if (parser->Parse(xmlForSMTKImporter(resource, token.first, token.second).c_str()) != 0)
      {
        server->proxyManager()->GetProxyDefinitionManager()->LoadConfigurationXML(
          parser->GetRootElement());
      }
    }
  }
}

// TODO: Once ParaView has the ability to unregister configurations for
// readers, the logic below can be used to ensure that removed operations are
// safely removed from ParaView's File->Open() interface.
#if 0
void unregisterSMTKImporter(
  pqServer* server, const std::string& resource, const std::string& fileFilters)
{
  auto app = pqApplicationCore::instance();
  if (app)
  {
    std::vector<std::pair<std::string, std::string> > extensionsAndDescriptions;
    extensionsAndDescriptionsFromFileFilters(fileFilters, extensionsAndDescriptions);
    for (auto& token : extensionsAndDescriptions)
    {
      // TODO: ParaView's vtkSIProxyDefinitionManager needs the ability to
      // remove a configuration.
    }
  }
}
#endif
} // namespace

static pqSMTKRegisterImportersBehavior* g_instance = nullptr;

pqSMTKRegisterImportersBehavior::pqSMTKRegisterImportersBehavior(QObject* parent)
  : Superclass(parent)
{
  QObject::connect(
    pqSMTKBehavior::instance(),
    (void(pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) & pqSMTKBehavior::addedManagerOnServer,
    this,
    &pqSMTKRegisterImportersBehavior::constructImporters);
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

void pqSMTKRegisterImportersBehavior::constructImporters(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper)
  {
    return;
  }

  auto importerGroup = smtk::operation::ImporterGroup(wrapper->smtkOperationManager());
  for (const auto& resourceName : importerGroup.supportedResources())
  {
    for (auto index : importerGroup.operationsForResource(resourceName))
    {
      auto fileItemDef = importerGroup.fileItemDefinitionForOperation(index);
      registerSMTKImporter(server, resourceName, fileItemDef->getFileFilters());
    }
  }

  std::weak_ptr<smtk::operation::Manager> opManager = wrapper->smtkOperationManager();
  wrapper->smtkOperationManager()->groupObservers().insert(
    [opManager, server](
      const smtk::operation::Operation::Index& index, const std::string& groupName, bool adding) {
      if (!adding)
      {
        return;
      }

      if (auto operationManager = opManager.lock())
      {
        if (groupName == smtk::operation::ImporterGroup::type_name)
        {
          auto importerGroup = smtk::operation::ImporterGroup(operationManager);
          assert(importerGroup.contains(index));
          auto fileItemDef = importerGroup.fileItemDefinitionForOperation(index);
          registerSMTKImporter(
            server, importerGroup.resourceForOperation(index), fileItemDef->getFileFilters());
        }
      }
    });

// TODO: Once ParaView has the ability to unregister configurations for
// readers, the logic below can be used to ensure that removed operations are
// safely removed from ParaView's File->Open() interface.
#if 0
  wrapper->smtkOperationManager()->groupObservers().insert(
    [opManager, server](const smtk::operation::Operation::Index& index,
                        const std::string& groupName, bool adding)
    {
      if (adding)
      {
        return;
      }

      if (auto operationManager = opManager.lock())
      {
        if (groupName == smtk::operation::ImporterGroup::type_name)
        {
          auto importerGroup = smtk::operation::ImporterGroup(operationManager);
          assert(importerGroup.contains(index));
          auto fileItemDef = importerGroup.fileItemDefinitionForOperation(index);
          unregisterSMTKImporter(server, importerGroup.resourceForOperation(index),
                                 fileItemDef->getFileFilters());
        }
      }
    });
#endif
}
