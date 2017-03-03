//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_ExportSMTKModel_h
#define pybind_smtk_model_operators_ExportSMTKModel_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/ExportSMTKModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::ExportSMTKModel, smtk::model::Operator > pybind11_init_smtk_model_ExportSMTKModel(py::module &m)
{
  PySharedPtrClass< smtk::model::ExportSMTKModel, smtk::model::Operator > instance(m, "ExportSMTKModel");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::ExportSMTKModel const &>())
    .def("deepcopy", (smtk::model::ExportSMTKModel & (smtk::model::ExportSMTKModel::*)(::smtk::model::ExportSMTKModel const &)) &smtk::model::ExportSMTKModel::operator=)
    .def_static("baseCreate", &smtk::model::ExportSMTKModel::baseCreate)
    .def("className", &smtk::model::ExportSMTKModel::className)
    .def("classname", &smtk::model::ExportSMTKModel::classname)
    .def_static("create", (std::shared_ptr<smtk::model::ExportSMTKModel> (*)()) &smtk::model::ExportSMTKModel::create)
    .def_static("create", (std::shared_ptr<smtk::model::ExportSMTKModel> (*)(::std::shared_ptr<smtk::model::ExportSMTKModel> &)) &smtk::model::ExportSMTKModel::create, py::arg("ref"))
    .def("name", &smtk::model::ExportSMTKModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::ExportSMTKModel> (smtk::model::ExportSMTKModel::*)() const) &smtk::model::ExportSMTKModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::ExportSMTKModel> (smtk::model::ExportSMTKModel::*)()) &smtk::model::ExportSMTKModel::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::ExportSMTKModel::operatorName)
    ;
  return instance;
}

#endif
