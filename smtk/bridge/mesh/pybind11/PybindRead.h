//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_Read_h
#define pybind_smtk_bridge_mesh_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/Read.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::Read, smtk::operation::XMLOperation > pybind11_init_smtk_bridge_mesh_Read(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::Read, smtk::operation::XMLOperation > instance(m, "Read");
  instance
    .def(py::init<::smtk::bridge::mesh::Read const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::mesh::Read & (smtk::bridge::mesh::Read::*)(::smtk::bridge::mesh::Read const &)) &smtk::bridge::mesh::Read::operator=)
    .def("classname", &smtk::bridge::mesh::Read::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Read> (*)()) &smtk::bridge::mesh::Read::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Read> (*)(::std::shared_ptr<smtk::bridge::mesh::Read> &)) &smtk::bridge::mesh::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::Read> (smtk::bridge::mesh::Read::*)()) &smtk::bridge::mesh::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::Read> (smtk::bridge::mesh::Read::*)() const) &smtk::bridge::mesh::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::mesh::read, "", py::arg("filePath"));

  return instance;
}

#endif
