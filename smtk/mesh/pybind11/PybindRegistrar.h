//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Registrar_h
#define pybind_smtk_mesh_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/resource/Registrar.h"

namespace py = pybind11;

inline py::class_< smtk::mesh::Registrar > pybind11_init_smtk_mesh_Registrar(py::module &m)
{
  py::class_< smtk::mesh::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::mesh::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::mesh::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::mesh::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::mesh::Registrar::unregisterFrom)
    ;
  return instance;
}

#endif
