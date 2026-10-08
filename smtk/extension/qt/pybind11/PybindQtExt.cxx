//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindShellQueue.h"
#include "PybindContainerQueue.h"

#include <QObject>

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindQtExt, qmod)
{
  qmod.doc() = "User interface components for SMTK in Qt.";

  py::module::import("smtk.common");
  py::module::import("smtk.operation");
  py::module::import("smtk.resource");
  py::module::import("smtk.task");
  py::module::import("smtk.job");
  py::module::import("smtk.view");

  auto qobj = py::class_< QObject >(qmod, "QObject");
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  auto shellQueue = pybind11_init_smtk_qt_job_ShellQueue(qmod);
  auto containerQueue = pybind11_init_smtk_qt_job_ContainerQueue(qmod);
}
