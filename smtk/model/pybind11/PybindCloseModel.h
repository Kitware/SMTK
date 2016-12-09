//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_CloseModel_h
#define pybind_smtk_model_operators_CloseModel_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/CloseModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::CloseModel, smtk::model::Operator > pybind11_init_smtk_model_CloseModel(py::module &m)
{
  PySharedPtrClass< smtk::model::CloseModel, smtk::model::Operator > instance(m, "CloseModel");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::CloseModel const &>())
    .def("deepcopy", (smtk::model::CloseModel & (smtk::model::CloseModel::*)(::smtk::model::CloseModel const &)) &smtk::model::CloseModel::operator=)
    .def("ableToOperate", &smtk::model::CloseModel::ableToOperate)
    .def_static("baseCreate", &smtk::model::CloseModel::baseCreate)
    .def("className", &smtk::model::CloseModel::className)
    .def("classname", &smtk::model::CloseModel::classname)
    .def_static("create", (std::shared_ptr<smtk::model::CloseModel> (*)()) &smtk::model::CloseModel::create)
    .def_static("create", (std::shared_ptr<smtk::model::CloseModel> (*)(::std::shared_ptr<smtk::model::CloseModel> &)) &smtk::model::CloseModel::create, py::arg("ref"))
    .def("name", &smtk::model::CloseModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::CloseModel> (smtk::model::CloseModel::*)() const) &smtk::model::CloseModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::CloseModel> (smtk::model::CloseModel::*)()) &smtk::model::CloseModel::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::CloseModel::operatorName)
    ;
  return instance;
}

#endif
