//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_ElevateMesh_h
#define pybind_smtk_mesh_operators_ElevateMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/ElevateMesh.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::ElevateMesh, smtk::operation::XMLOperator > pybind11_init_smtk_mesh_ElevateMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ElevateMesh, smtk::operation::XMLOperator > instance(m, "ElevateMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::ElevateMesh const &>())
    .def("deepcopy", (smtk::mesh::ElevateMesh & (smtk::mesh::ElevateMesh::*)(::smtk::mesh::ElevateMesh const &)) &smtk::mesh::ElevateMesh::operator=)
    .def("ableToOperate", &smtk::mesh::ElevateMesh::ableToOperate)
    .def("classname", &smtk::mesh::ElevateMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::ElevateMesh> (*)()) &smtk::mesh::ElevateMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::ElevateMesh> (*)(::std::shared_ptr<smtk::mesh::ElevateMesh> &)) &smtk::mesh::ElevateMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::ElevateMesh> (smtk::mesh::ElevateMesh::*)() const) &smtk::mesh::ElevateMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::ElevateMesh> (smtk::mesh::ElevateMesh::*)()) &smtk::mesh::ElevateMesh::shared_from_this)
    ;
  return instance;
}

#endif
