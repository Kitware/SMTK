//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/RegisterPythonResource.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/resource/Metadata.h"
#include "smtk/resource/pybind11/PyResource.h"

#include "smtk/Regex.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <iostream>
#include <string>

namespace
{
bool importResource(
  smtk::resource::Manager& manager,
  const std::string& moduleName,
  const std::string& resourceName)
{
  std::string typeName = moduleName + "." + resourceName;
  smtk::resource::Resource::Index index = std::hash<std::string>{}(typeName);
  auto create = [moduleName, resourceName, index](
                  const smtk::common::UUID& uid,
                  const std::shared_ptr<smtk::common::Managers>& managers) {
    (void)managers;
    auto resource = smtk::resource::PyResource::create(moduleName, resourceName, index);
    if (resource)
    {
      resource->setId(uid);
    }
    return resource;
  };

  return manager.registerResource(
    smtk::resource::Metadata(typeName, index, {}, create, nullptr, nullptr));
}

std::vector<std::string> importResourcesFromModule(
  const std::string& moduleName,
  smtk::resource::Manager& manager)
{
  // Query the module for SMTK resources
  std::stringstream cmd;
  cmd << "import sys, inspect, smtk, smtk.resource, " << moduleName << "\n"
      << "ops = set()\n"
      << "for name, obj in inspect.getmembers(" << moduleName << "):\n"
      << "    if inspect.isclass(obj) and issubclass(obj, smtk.resource.Resource):\n"
      << "        ops.add(obj.__name__)\n"
      << "resourcestring = ';;'.join(str(op) for op in ops)\n";

  pybind11::dict locals;
  pybind11::exec(cmd.str().c_str(), pybind11::globals(), locals);

  std::string resourceNames = locals["resourcestring"].cast<std::string>();

  if (resourceNames.empty())
  {
    // There were no resources in the module
    return std::vector<std::string>();
  }

  // As per the above python snippet, the output is a string of all of the
  // resource names defined in the input file, separated by ";;". We parse this
  // string to loop over each python resource.
  smtk::regex re(";;");
  smtk::sregex_token_iterator first{ resourceNames.begin(), resourceNames.end(), re, -1 }, last;

  std::vector<std::string> typeNames;

  // For each resource name in the imported module...
  while (first != last)
  {
    const std::string& resourceName = *first++;

    bool registered = ::importResource(manager, moduleName, resourceName);

    if (registered)
    {
      typeNames.emplace_back(moduleName + "." + resourceName);
    }
  }

  return typeNames;
}
} // namespace

namespace smtk
{
namespace resource
{

bool registerPythonResource(
  const smtk::resource::Manager::Ptr& resourceManager,
  const std::string& moduleName)
{
  if (resourceManager == nullptr)
  {
    std::cerr << "WARNING: cannot register module \"" << moduleName
              << "\" because the resource manager is not valid." << std::endl;
    return false;
  }

  smtk::common::PythonInterpreter::instance().initialize();
  if (!smtk::common::PythonInterpreter::instance().canFindModule(moduleName))
  {
    std::cerr << "WARNING: module \"" << moduleName
              << "\" has been requested but cannot be imported." << std::endl;
    std::cerr << std::endl << "Paths searched:" << std::endl;
    auto paths = smtk::common::PythonInterpreter::instance().pythonPath();
    for (const auto& path : paths)
    {
      std::cerr << path << std::endl;
    }
    return false;
  }
  else
  {
    pybind11::module mod = pybind11::module::import(moduleName.c_str());
    auto resourceNames = ::importResourcesFromModule(moduleName, *resourceManager);
    if (resourceNames.empty())
    {
      std::cerr << "WARNING: module \"" << moduleName
                << "\" has been imported but contains no resources." << std::endl;
      return false;
    }
    return true;
  }
}
} // namespace resource
} // namespace smtk
