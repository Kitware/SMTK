//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_WriteMesh_h
#define pybind_smtk_model_operators_WriteMesh_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/WriteMesh.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::WriteMesh, smtk::model::Operator > pybind11_init_smtk_model_WriteMesh(py::module &m)
{
  PySharedPtrClass< smtk::model::WriteMesh, smtk::model::Operator > instance(m, "WriteMesh", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::WriteMesh const &>())
    .def("deepcopy", (smtk::model::WriteMesh & (smtk::model::WriteMesh::*)(::smtk::model::WriteMesh const &)) &smtk::model::WriteMesh::operator=)
    .def("ableToOperate", &smtk::model::WriteMesh::ableToOperate)
    .def_static("baseCreate", &smtk::model::WriteMesh::baseCreate)
    .def("className", &smtk::model::WriteMesh::className)
    .def("classname", &smtk::model::WriteMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::model::WriteMesh> (*)()) &smtk::model::WriteMesh::create)
    .def_static("create", (std::shared_ptr<smtk::model::WriteMesh> (*)(::std::shared_ptr<smtk::model::WriteMesh> &)) &smtk::model::WriteMesh::create, py::arg("ref"))
    .def("name", &smtk::model::WriteMesh::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::WriteMesh> (smtk::model::WriteMesh::*)() const) &smtk::model::WriteMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::WriteMesh> (smtk::model::WriteMesh::*)()) &smtk::model::WriteMesh::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::WriteMesh::operatorName)
    ;
  return instance;
}

#endif
