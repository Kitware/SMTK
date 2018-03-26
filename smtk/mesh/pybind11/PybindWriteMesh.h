//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_WriteMesh_h
#define pybind_smtk_mesh_operators_WriteMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/WriteMesh.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::WriteMesh, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_WriteMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::WriteMesh, smtk::operation::XMLOperation > instance(m, "WriteMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::WriteMesh const &>())
    .def("deepcopy", (smtk::mesh::WriteMesh & (smtk::mesh::WriteMesh::*)(::smtk::mesh::WriteMesh const &)) &smtk::mesh::WriteMesh::operator=)
    .def("ableToOperate", &smtk::mesh::WriteMesh::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::WriteMesh> (*)()) &smtk::mesh::WriteMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::WriteMesh> (*)(::std::shared_ptr<smtk::mesh::WriteMesh> &)) &smtk::mesh::WriteMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::WriteMesh> (smtk::mesh::WriteMesh::*)() const) &smtk::mesh::WriteMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::WriteMesh> (smtk::mesh::WriteMesh::*)()) &smtk::mesh::WriteMesh::shared_from_this)
    ;
  return instance;
}

#endif
