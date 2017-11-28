//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/environment/Environment.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

namespace py = pybind11;

template <typename T, typename... Args>
using PySingletonClass = py::class_<T, std::unique_ptr<T, py::nodelete>, Args...>;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindEnvironment, environment)
{
  environment.doc() = "<description>";

  PySingletonClass<smtk::environment::OperationManager> omInstance(environment, "OperationManager");
    omInstance.def_static("instance", &smtk::environment::OperationManager::instance, pybind11::return_value_policy::reference);

  PySingletonClass<smtk::environment::ResourceManager> rmInstance(environment, "ResourceManager");
  rmInstance.def_static("instance", &smtk::environment::ResourceManager::instance, pybind11::return_value_policy::reference);
}
