//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Environment_h
#define pybind_smtk_common_Environment_h

#include <pybind11/pybind11.h>

#include "smtk/common/Environment.h"

namespace py = pybind11;

py::class_< smtk::common::Environment > pybind11_init_smtk_common_Environment(py::module &m)
{
  py::class_< smtk::common::Environment > instance(m, "Environment");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::Environment const &>())
    .def("deepcopy", (smtk::common::Environment & (smtk::common::Environment::*)(::smtk::common::Environment const &)) &smtk::common::Environment::operator=)
    .def_static("hasVariable", &smtk::common::Environment::hasVariable, py::arg("varName"))
    .def_static("getVariable", &smtk::common::Environment::getVariable, py::arg("varName"))
    .def_static("setVariable", &smtk::common::Environment::setVariable, py::arg("varName"), py::arg("value"))
    ;
  return instance;
}

#endif
