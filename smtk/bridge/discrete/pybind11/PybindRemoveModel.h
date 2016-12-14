//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_RemoveModel_h
#define pybind_smtk_bridge_discrete_operators_RemoveModel_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/RemoveModel.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::RemoveModel, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_RemoveModel(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::RemoveModel, smtk::model::Operator > instance(m, "RemoveModel");
  instance
    .def(py::init<::smtk::bridge::discrete::RemoveModel const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::discrete::RemoveModel & (smtk::bridge::discrete::RemoveModel::*)(::smtk::bridge::discrete::RemoveModel const &)) &smtk::bridge::discrete::RemoveModel::operator=)
    .def("classname", &smtk::bridge::discrete::RemoveModel::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::RemoveModel> (*)()) &smtk::bridge::discrete::RemoveModel::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::RemoveModel> (*)(::std::shared_ptr<smtk::bridge::discrete::RemoveModel> &)) &smtk::bridge::discrete::RemoveModel::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::RemoveModel> (smtk::bridge::discrete::RemoveModel::*)()) &smtk::bridge::discrete::RemoveModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::RemoveModel> (smtk::bridge::discrete::RemoveModel::*)() const) &smtk::bridge::discrete::RemoveModel::shared_from_this)
    .def("name", &smtk::bridge::discrete::RemoveModel::name)
    .def("className", &smtk::bridge::discrete::RemoveModel::className)
    .def_static("baseCreate", &smtk::bridge::discrete::RemoveModel::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::RemoveModel::ableToOperate)
    ;
  return instance;
}

#endif
