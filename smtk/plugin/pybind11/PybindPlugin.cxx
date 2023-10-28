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

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Manager.h"

#include "smtk/plugin/Manager.txx"

#include <memory>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindPlugin, plugin)
{
  // We need to explicitly list all of the managers we want our plugin manager
  // to accept. I'm sure there's a way to do this more elegantly, but for now I
  // am ok with this being our payment for plugin registration in python (which
  // has full access to the "environment" managers anyway). This module is
  // really only supposed to be used for testing; it may move into the testing
  // directory.
  plugin
    .def("registerPluginsTo",
      [](const std::shared_ptr<smtk::common::Managers>& managers) {
        smtk::plugin::Manager::instance()->registerPluginsTo(managers);
      })
    .def("unregisterPluginsFrom",
      [](const std::shared_ptr<smtk::common::Managers>& managers) {
        smtk::plugin::Manager::instance()->unregisterPluginsFrom(managers);
      })
    .def("registerPluginsTo",
      [](const std::shared_ptr<smtk::resource::Manager>& manager) {
        smtk::plugin::Manager::instance()->registerPluginsTo(manager);
      })
    .def("unregisterPluginsFrom",
      [](const std::shared_ptr<smtk::resource::Manager>& manager) {
        smtk::plugin::Manager::instance()->unregisterPluginsFrom(manager);
      })
    .def("registerPluginsTo",
      [](const std::shared_ptr<smtk::operation::Manager>& manager) {
        smtk::plugin::Manager::instance()->registerPluginsTo(manager);
      })
    .def("unregisterPluginsFrom", [](const std::shared_ptr<smtk::operation::Manager>& manager) {
      smtk::plugin::Manager::instance()->unregisterPluginsFrom(manager);
    })
    .def("registerPluginsTo",
      [](const std::shared_ptr<smtk::task::Manager>& manager) {
        smtk::plugin::Manager::instance()->registerPluginsTo(manager);
      })
    .def("unregisterPluginsFrom",
      [](const std::shared_ptr<smtk::task::Manager>& manager) {
        smtk::plugin::Manager::instance()->unregisterPluginsFrom(manager);
      })
    ;
}
