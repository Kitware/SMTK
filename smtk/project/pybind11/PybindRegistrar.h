//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Registrar_h
#define pybind_smtk_project_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/project/Registrar.h"

namespace py = pybind11;

inline py::class_< smtk::project::Registrar > pybind11_init_smtk_project_Registrar(py::module &m)
{
  py::class_< smtk::project::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::Registrar const &>())
    .def("deepcopy", (smtk::project::Registrar & (smtk::project::Registrar::*)(::smtk::project::Registrar const &)) &smtk::project::Registrar::operator=)
    .def_static("registerTo", (void (*)(::smtk::common::Managers::Ptr const &)) &smtk::project::Registrar::registerTo, py::arg("arg0"))
    .def_static("registerTo", (void (*)(::smtk::operation::Manager::Ptr const &)) &smtk::project::Registrar::registerTo, py::arg("arg0"))
    .def_static("registerTo", (void (*)(::smtk::project::Manager::Ptr const &)) &smtk::project::Registrar::registerTo, py::arg("arg0"))
    .def_static("registerTo", (void (*)(::smtk::resource::Manager::Ptr const &)) &smtk::project::Registrar::registerTo, py::arg("arg0"))
    .def_static("registerTo", (void (*)(::smtk::view::Manager::Ptr const &)) &smtk::project::Registrar::registerTo, py::arg("arg0"))
    .def_static("unregisterFrom", (void (*)(::smtk::common::Managers::Ptr const &)) &smtk::project::Registrar::unregisterFrom, py::arg("arg0"))
    .def_static("unregisterFrom", (void (*)(::smtk::operation::Manager::Ptr const &)) &smtk::project::Registrar::unregisterFrom, py::arg("arg0"))
    .def_static("unregisterFrom", (void (*)(::smtk::project::Manager::Ptr const &)) &smtk::project::Registrar::unregisterFrom, py::arg("arg0"))
    .def_static("unregisterFrom", (void (*)(::smtk::resource::Manager::Ptr const &)) &smtk::project::Registrar::unregisterFrom, py::arg("arg0"))
    .def_static("unregisterFrom", (void (*)(::smtk::view::Manager::Ptr const &)) &smtk::project::Registrar::unregisterFrom, py::arg("arg0"))
    ;
  return instance;
}

#endif
