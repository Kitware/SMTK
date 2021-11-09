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
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindpqSMTKBehavior.h"
#include "PybindpqSMTKResourceRepresentation.h"
#include "PybindpqSMTKResource.h"
#include "PybindpqSMTKOperationPanel.h"
#include "PybindpqSMTKResourcePanel.h"
#include "PybindpqSMTKResourceBrowser.h"
#include "PybindpqSMTKWrapper.h"
#include "PybindpqSMTKAttributePanel.h"

#include <QObject>

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindPQComponents, pqcomponents)
{
  pqcomponents.doc() = "User interface components for SMTK in ParaView.";

  py::module::import("smtk.common");
  py::module::import("smtk.operation");
  py::module::import("smtk.resource");
  py::module::import("smtk.task");
  py::module::import("smtk.view");

  auto qobj = py::class_< QObject >(pqcomponents, "QObject");
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< pqSMTKBehavior, QObject > pqSMTKBehavior = pybind11_init_pqSMTKBehavior(pqcomponents);
}
