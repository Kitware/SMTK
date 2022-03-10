//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/RegisterPythonProject.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/project/Metadata.h"
#include "smtk/project/pybind11/PyProject.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

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
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
#endif

#include <iostream>
#include <string>

namespace
{
bool importProject(
  smtk::project::Manager& manager,
  const std::string& moduleName,
  const std::string& projectName)
{
  std::string typeName = moduleName + "." + projectName;
  smtk::project::Project::Index index = std::hash<std::string>{}(typeName);
  auto create = [moduleName, projectName, index](
                  const smtk::common::UUID& uid,
                  const std::shared_ptr<smtk::common::Managers>& managers)
    -> std::shared_ptr<smtk::project::Project> {
    return smtk::project::PyProject::create(moduleName, projectName, index, uid, managers);
  };

  return manager.registerProject(smtk::project::Metadata(typeName, index, create));
}

std::vector<std::string> importProjectsFromModule(
  const std::string& moduleName,
  smtk::project::Manager& manager)
{
  // Query the module for SMTK projects
  std::stringstream cmd;
  cmd << "import sys, inspect, smtk, smtk.project, " << moduleName << "\n"
      << "ops = set()\n"
      << "for name, obj in inspect.getmembers(" << moduleName << "):\n"
      << "    if inspect.isclass(obj) and issubclass(obj, smtk.project.Project):\n"
      << "        ops.add(obj.__name__)\n"
      << "projectstring = ';;'.join(str(op) for op in ops)\n";

  pybind11::dict locals;
  pybind11::exec(cmd.str().c_str(), pybind11::globals(), locals);

  std::string projectNames = locals["projectstring"].cast<std::string>();

  if (projectNames.empty())
  {
    // There were no projects in the module
    return std::vector<std::string>();
  }

  // As per the above python snippet, the output is a string of all of the
  // project names defined in the input file, separated by ";;". We parse this
  // string to loop over each python project.
  regex re(";;");
  sregex_token_iterator first{ projectNames.begin(), projectNames.end(), re, -1 }, last;

  std::vector<std::string> typeNames;

  // For each project name in the imported module...
  while (first != last)
  {
    const std::string& projectName = *first++;

    bool registered = ::importProject(manager, moduleName, projectName);

    if (registered)
    {
      typeNames.emplace_back(moduleName + "." + projectName);
    }
  }

  return typeNames;
}
} // namespace

namespace smtk
{
namespace project
{

bool registerPythonProject(
  const smtk::project::Manager::Ptr& projectManager,
  const std::string& moduleName)
{
  if (projectManager == nullptr)
  {
    std::cerr << "WARNING: cannot register module \"" << moduleName
              << "\" because the project manager is not valid." << std::endl;
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
    auto projectNames = ::importProjectsFromModule(moduleName, *projectManager);
    if (projectNames.empty())
    {
      std::cerr << "WARNING: module \"" << moduleName
                << "\" has been imported but contains no projects." << std::endl;
      return false;
    }
    return true;
  }
}
} // namespace project
} // namespace smtk
