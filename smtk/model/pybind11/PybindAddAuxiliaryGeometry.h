//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_AddAuxiliaryGeometry_h
#define pybind_smtk_model_operators_AddAuxiliaryGeometry_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::AddAuxiliaryGeometry, smtk::model::Operator > pybind11_init_smtk_model_AddAuxiliaryGeometry(py::module &m)
{
  PySharedPtrClass< smtk::model::AddAuxiliaryGeometry, smtk::model::Operator > instance(m, "AddAuxiliaryGeometry", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::AddAuxiliaryGeometry const &>())
    .def("deepcopy", (smtk::model::AddAuxiliaryGeometry & (smtk::model::AddAuxiliaryGeometry::*)(::smtk::model::AddAuxiliaryGeometry const &)) &smtk::model::AddAuxiliaryGeometry::operator=)
    .def_static("baseCreate", &smtk::model::AddAuxiliaryGeometry::baseCreate)
    .def("className", &smtk::model::AddAuxiliaryGeometry::className)
    .def("classname", &smtk::model::AddAuxiliaryGeometry::classname)
    .def_static("create", (std::shared_ptr<smtk::model::AddAuxiliaryGeometry> (*)()) &smtk::model::AddAuxiliaryGeometry::create)
    .def_static("create", (std::shared_ptr<smtk::model::AddAuxiliaryGeometry> (*)(::std::shared_ptr<smtk::model::AddAuxiliaryGeometry> &)) &smtk::model::AddAuxiliaryGeometry::create, py::arg("ref"))
    .def("name", &smtk::model::AddAuxiliaryGeometry::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::AddAuxiliaryGeometry> (smtk::model::AddAuxiliaryGeometry::*)() const) &smtk::model::AddAuxiliaryGeometry::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::AddAuxiliaryGeometry> (smtk::model::AddAuxiliaryGeometry::*)()) &smtk::model::AddAuxiliaryGeometry::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::AddAuxiliaryGeometry::operatorName)
    ;
  return instance;
}

#endif
