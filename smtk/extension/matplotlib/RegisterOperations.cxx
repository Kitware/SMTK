//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/matplotlib/RegisterOperations.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/operation/ImportPythonOperator.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <iostream>

namespace smtk
{
namespace extension
{
namespace matplotlib
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  if (operationManager == nullptr)
  {
    return;
  }

  std::vector<std::string> moduleNames = { "smtk.extension.matplotlib.render_mesh" };

  for (auto& moduleName : moduleNames)
  {
    smtk::common::PythonInterpreter::instance().initialize();
    if (!smtk::common::PythonInterpreter::instance().canFindModule(moduleName))
    {
      std::cerr << "WARNING: module \"" << moduleName
                << "\" has been requested but cannot be imported." << std::endl;
      std::cerr << std::endl << "Paths searched:" << std::endl;
      auto paths = smtk::common::PythonInterpreter::instance().pythonPath();
      for (auto path : paths)
      {
        std::cerr << path << std::endl;
      }
    }
    else
    {
      pybind11::module mod = pybind11::module::import(moduleName.c_str());
      auto opNames = smtk::operation::ImportPythonOperator::importOperatorsFromModule(
        moduleName, *operationManager);
      if (opNames.empty())
      {
        std::cerr << "WARNING: module \"" << moduleName
                  << "\" has been imported but contains no operators." << std::endl;
      }
    }
  }
}
}
}
}
