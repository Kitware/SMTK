//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_DeleteMesh_h
#define pybind_smtk_model_operators_DeleteMesh_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/DeleteMesh.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::DeleteMesh, smtk::model::Operator > pybind11_init_smtk_model_DeleteMesh(py::module &m)
{
  PySharedPtrClass< smtk::model::DeleteMesh, smtk::model::Operator > instance(m, "DeleteMesh", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::DeleteMesh const &>())
    .def("deepcopy", (smtk::model::DeleteMesh & (smtk::model::DeleteMesh::*)(::smtk::model::DeleteMesh const &)) &smtk::model::DeleteMesh::operator=)
    .def("ableToOperate", &smtk::model::DeleteMesh::ableToOperate)
    .def_static("baseCreate", &smtk::model::DeleteMesh::baseCreate)
    .def("className", &smtk::model::DeleteMesh::className)
    .def("classname", &smtk::model::DeleteMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::model::DeleteMesh> (*)()) &smtk::model::DeleteMesh::create)
    .def_static("create", (std::shared_ptr<smtk::model::DeleteMesh> (*)(::std::shared_ptr<smtk::model::DeleteMesh> &)) &smtk::model::DeleteMesh::create, py::arg("ref"))
    .def("name", &smtk::model::DeleteMesh::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::DeleteMesh> (smtk::model::DeleteMesh::*)() const) &smtk::model::DeleteMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::DeleteMesh> (smtk::model::DeleteMesh::*)()) &smtk::model::DeleteMesh::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::DeleteMesh::operatorName)
    ;
  return instance;
}

#endif
