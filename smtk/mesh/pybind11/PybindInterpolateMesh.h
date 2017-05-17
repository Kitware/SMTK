//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_InterpolateMesh_h
#define pybind_smtk_mesh_operators_InterpolateMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/InterpolateMesh.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::InterpolateMesh, smtk::model::Operator > pybind11_init_smtk_mesh_InterpolateMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::InterpolateMesh, smtk::model::Operator > instance(m, "InterpolateMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::InterpolateMesh const &>())
    .def("deepcopy", (smtk::mesh::InterpolateMesh & (smtk::mesh::InterpolateMesh::*)(::smtk::mesh::InterpolateMesh const &)) &smtk::mesh::InterpolateMesh::operator=)
    .def("ableToOperate", &smtk::mesh::InterpolateMesh::ableToOperate)
    .def_static("baseCreate", &smtk::mesh::InterpolateMesh::baseCreate)
    .def("className", &smtk::mesh::InterpolateMesh::className)
    .def("classname", &smtk::mesh::InterpolateMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::InterpolateMesh> (*)()) &smtk::mesh::InterpolateMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::InterpolateMesh> (*)(::std::shared_ptr<smtk::mesh::InterpolateMesh> &)) &smtk::mesh::InterpolateMesh::create, py::arg("ref"))
    .def("name", &smtk::mesh::InterpolateMesh::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::InterpolateMesh> (smtk::mesh::InterpolateMesh::*)() const) &smtk::mesh::InterpolateMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::InterpolateMesh> (smtk::mesh::InterpolateMesh::*)()) &smtk::mesh::InterpolateMesh::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::mesh::InterpolateMesh::operatorName)
    ;
  return instance;
}

#endif
