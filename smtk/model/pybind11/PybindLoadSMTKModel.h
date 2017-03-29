//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_LoadSMTKModel_h
#define pybind_smtk_model_operators_LoadSMTKModel_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/LoadSMTKModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::LoadSMTKModel, smtk::model::Operator > pybind11_init_smtk_model_LoadSMTKModel(py::module &m)
{
  PySharedPtrClass< smtk::model::LoadSMTKModel, smtk::model::Operator > instance(m, "LoadSMTKModel");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::LoadSMTKModel const &>())
    .def("deepcopy", (smtk::model::LoadSMTKModel & (smtk::model::LoadSMTKModel::*)(::smtk::model::LoadSMTKModel const &)) &smtk::model::LoadSMTKModel::operator=)
    .def_static("baseCreate", &smtk::model::LoadSMTKModel::baseCreate)
    .def("className", &smtk::model::LoadSMTKModel::className)
    .def("classname", &smtk::model::LoadSMTKModel::classname)
    .def_static("create", (std::shared_ptr<smtk::model::LoadSMTKModel> (*)()) &smtk::model::LoadSMTKModel::create)
    .def_static("create", (std::shared_ptr<smtk::model::LoadSMTKModel> (*)(::std::shared_ptr<smtk::model::LoadSMTKModel> &)) &smtk::model::LoadSMTKModel::create, py::arg("ref"))
    .def("name", &smtk::model::LoadSMTKModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::LoadSMTKModel> (smtk::model::LoadSMTKModel::*)() const) &smtk::model::LoadSMTKModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::LoadSMTKModel> (smtk::model::LoadSMTKModel::*)()) &smtk::model::LoadSMTKModel::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::LoadSMTKModel::operatorName)
    ;
  return instance;
}

#endif
