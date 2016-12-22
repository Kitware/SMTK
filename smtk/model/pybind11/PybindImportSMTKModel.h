//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_ImportSMTKModel_h
#define pybind_smtk_model_operators_ImportSMTKModel_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/ImportSMTKModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::ImportSMTKModel, smtk::model::Operator > pybind11_init_smtk_model_ImportSMTKModel(py::module &m)
{
  PySharedPtrClass< smtk::model::ImportSMTKModel, smtk::model::Operator > instance(m, "ImportSMTKModel", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::ImportSMTKModel const &>())
    .def("deepcopy", (smtk::model::ImportSMTKModel & (smtk::model::ImportSMTKModel::*)(::smtk::model::ImportSMTKModel const &)) &smtk::model::ImportSMTKModel::operator=)
    .def_static("baseCreate", &smtk::model::ImportSMTKModel::baseCreate)
    .def("className", &smtk::model::ImportSMTKModel::className)
    .def("classname", &smtk::model::ImportSMTKModel::classname)
    .def_static("create", (std::shared_ptr<smtk::model::ImportSMTKModel> (*)()) &smtk::model::ImportSMTKModel::create)
    .def_static("create", (std::shared_ptr<smtk::model::ImportSMTKModel> (*)(::std::shared_ptr<smtk::model::ImportSMTKModel> &)) &smtk::model::ImportSMTKModel::create, py::arg("ref"))
    .def("name", &smtk::model::ImportSMTKModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::ImportSMTKModel> (smtk::model::ImportSMTKModel::*)() const) &smtk::model::ImportSMTKModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::ImportSMTKModel> (smtk::model::ImportSMTKModel::*)()) &smtk::model::ImportSMTKModel::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::ImportSMTKModel::operatorName)
    ;
  return instance;
}

#endif
