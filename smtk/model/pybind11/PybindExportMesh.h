//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_ExportMesh_h
#define pybind_smtk_model_operators_ExportMesh_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/ExportMesh.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::ExportMesh, smtk::model::Operator > pybind11_init_smtk_model_ExportMesh(py::module &m)
{
  PySharedPtrClass< smtk::model::ExportMesh, smtk::model::Operator > instance(m, "ExportMesh", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::ExportMesh const &>())
    .def("deepcopy", (smtk::model::ExportMesh & (smtk::model::ExportMesh::*)(::smtk::model::ExportMesh const &)) &smtk::model::ExportMesh::operator=)
    .def("ableToOperate", &smtk::model::ExportMesh::ableToOperate)
    .def_static("baseCreate", &smtk::model::ExportMesh::baseCreate)
    .def("className", &smtk::model::ExportMesh::className)
    .def("classname", &smtk::model::ExportMesh::classname)
    .def_static("create", (std::shared_ptr<smtk::model::ExportMesh> (*)()) &smtk::model::ExportMesh::create)
    .def_static("create", (std::shared_ptr<smtk::model::ExportMesh> (*)(::std::shared_ptr<smtk::model::ExportMesh> &)) &smtk::model::ExportMesh::create, py::arg("ref"))
    .def("name", &smtk::model::ExportMesh::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::ExportMesh> (smtk::model::ExportMesh::*)() const) &smtk::model::ExportMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::ExportMesh> (smtk::model::ExportMesh::*)()) &smtk::model::ExportMesh::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::ExportMesh::operatorName)
    ;
  return instance;
}

#endif
