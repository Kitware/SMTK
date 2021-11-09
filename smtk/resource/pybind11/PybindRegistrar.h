//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Registrar_h
#define pybind_smtk_resource_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Registrar.h"

namespace py = pybind11;

inline py::class_< smtk::resource::Registrar > pybind11_init_smtk_resource_Registrar(py::module &m)
{
  py::class_< smtk::resource::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::Registrar const &>())
    .def("deepcopy", (smtk::resource::Registrar & (smtk::resource::Registrar::*)(::smtk::resource::Registrar const &)) &smtk::resource::Registrar::operator=)
    .def_static("registerTo", (void (*)(::smtk::common::Managers::Ptr const &)) &smtk::resource::Registrar::registerTo, py::arg("manager"))
    .def_static("unregisterFrom", (void (*)(::smtk::common::Managers::Ptr const &)) &smtk::resource::Registrar::unregisterFrom, py::arg("manager"))
    ;
  return instance;
}

#endif
