//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_DeleteMesh_h
#define pybind_smtk_mesh_operators_DeleteMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/DeleteMesh.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::DeleteMesh, smtk::operation::XMLOperator > pybind11_init_smtk_mesh_DeleteMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::DeleteMesh, smtk::operation::XMLOperator > instance(m, "DeleteMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::DeleteMesh const &>())
    .def("deepcopy", (smtk::mesh::DeleteMesh & (smtk::mesh::DeleteMesh::*)(::smtk::mesh::DeleteMesh const &)) &smtk::mesh::DeleteMesh::operator=)
    .def("ableToOperate", &smtk::mesh::DeleteMesh::ableToOperate)
    .def("classname", &smtk::mesh::DeleteMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::DeleteMesh> (*)()) &smtk::mesh::DeleteMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::DeleteMesh> (*)(::std::shared_ptr<smtk::mesh::DeleteMesh> &)) &smtk::mesh::DeleteMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::DeleteMesh> (smtk::mesh::DeleteMesh::*)() const) &smtk::mesh::DeleteMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::DeleteMesh> (smtk::mesh::DeleteMesh::*)()) &smtk::mesh::DeleteMesh::shared_from_this)
    ;
  return instance;
}

#endif
