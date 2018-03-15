//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_Write_h
#define pybind_smtk_bridge_mesh_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/Write.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::Write, smtk::operation::XMLOperation > pybind11_init_smtk_bridge_mesh_Write(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::Write, smtk::operation::XMLOperation > instance(m, "Write");
  instance
    .def(py::init<::smtk::bridge::mesh::Write const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::mesh::Write & (smtk::bridge::mesh::Write::*)(::smtk::bridge::mesh::Write const &)) &smtk::bridge::mesh::Write::operator=)
    .def("classname", &smtk::bridge::mesh::Write::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Write> (*)()) &smtk::bridge::mesh::Write::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Write> (*)(::std::shared_ptr<smtk::bridge::mesh::Write> &)) &smtk::bridge::mesh::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::Write> (smtk::bridge::mesh::Write::*)()) &smtk::bridge::mesh::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::Write> (smtk::bridge::mesh::Write::*)() const) &smtk::bridge::mesh::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr)) &smtk::bridge::mesh::write, "", py::arg("resource"));

  return instance;
}

#endif
