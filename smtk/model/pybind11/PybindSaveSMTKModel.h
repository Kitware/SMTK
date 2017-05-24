//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_SaveSMTKModel_h
#define pybind_smtk_model_operators_SaveSMTKModel_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/SaveSMTKModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::SaveSMTKModel, smtk::model::Operator > pybind11_init_smtk_model_SaveSMTKModel(py::module &m)
{
  PySharedPtrClass< smtk::model::SaveSMTKModel, smtk::model::Operator > instance(m, "SaveSMTKModel");
  instance
    .def("deepcopy", (smtk::model::SaveSMTKModel & (smtk::model::SaveSMTKModel::*)(::smtk::model::SaveSMTKModel const &)) &smtk::model::SaveSMTKModel::operator=)
    .def_static("baseCreate", &smtk::model::SaveSMTKModel::baseCreate)
    .def("className", &smtk::model::SaveSMTKModel::className)
    .def("classname", &smtk::model::SaveSMTKModel::classname)
    .def_static("create", (std::shared_ptr<smtk::model::SaveSMTKModel> (*)()) &smtk::model::SaveSMTKModel::create)
    .def_static("create", (std::shared_ptr<smtk::model::SaveSMTKModel> (*)(::std::shared_ptr<smtk::model::SaveSMTKModel> &)) &smtk::model::SaveSMTKModel::create, py::arg("ref"))
    .def("name", &smtk::model::SaveSMTKModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::SaveSMTKModel> (smtk::model::SaveSMTKModel::*)() const) &smtk::model::SaveSMTKModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::SaveSMTKModel> (smtk::model::SaveSMTKModel::*)()) &smtk::model::SaveSMTKModel::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::SaveSMTKModel::operatorName)
    ;
  return instance;
}

#endif
