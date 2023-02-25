//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Registrar_h
#define pybind_smtk_task_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/task/Registrar.h"

namespace py = pybind11;

inline py::class_< smtk::task::Registrar > pybind11_init_smtk_task_Registrar(py::module &m)
{
  py::class_< smtk::task::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::task::Registrar const &>())
    .def("deepcopy", (smtk::task::Registrar & (smtk::task::Registrar::*)(::smtk::task::Registrar const &)) &smtk::task::Registrar::operator=)
    .def_static("registerTo", (void (*)(::smtk::task::Manager::Ptr const &)) &smtk::task::Registrar::registerTo, py::arg("manager"))
    .def_static("unregisterFrom", (void (*)(::smtk::task::Manager::Ptr const &)) &smtk::task::Registrar::unregisterFrom, py::arg("manager"))
    ;
  return instance;
}

#endif
