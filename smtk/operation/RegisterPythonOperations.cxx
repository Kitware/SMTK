//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/RegisterPythonOperations.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/operation/operators/ImportPythonOperation.h"

#include "smtk/operation/pybind11/PyOperation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <iostream>
#include <string>

namespace smtk
{
namespace operation
{

std::function<void(PyOperation::SimpleFunction)> PyOperation::runOnMainThread =
  [](PyOperation::SimpleFunction fn) {
    if (fn)
    {
      fn();
    }
  };

bool registerPythonOperations(
  const smtk::operation::Manager::Ptr& operationManager,
  const std::string& moduleName)
{
  if (operationManager == nullptr)
  {
    std::cerr << "WARNING: cannot register module \"" << moduleName
              << "\" because the operation manager is not valid." << std::endl;
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
    auto opNames = smtk::operation::ImportPythonOperation::importOperationsFromModule(
      moduleName, *operationManager);
    if (opNames.empty())
    {
      std::cerr << "WARNING: module \"" << moduleName
                << "\" has been imported but contains no operations." << std::endl;
      return false;
    }
    return true;
  }
}
} // namespace operation
} // namespace smtk
