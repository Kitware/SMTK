//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_RemoveModel_h
#define pybind_smtk_bridge_cgm_operators_RemoveModel_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/RemoveModel.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::RemoveModel > pybind11_init_smtk_bridge_cgm_RemoveModel(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::RemoveModel > instance(m, "RemoveModel", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::RemoveModel const &>())
    .def("deepcopy", (smtk::bridge::cgm::RemoveModel & (smtk::bridge::cgm::RemoveModel::*)(::smtk::bridge::cgm::RemoveModel const &)) &smtk::bridge::cgm::RemoveModel::operator=)
    .def("ableToOperate", &smtk::bridge::cgm::RemoveModel::ableToOperate)
    .def_static("baseCreate", &smtk::bridge::cgm::RemoveModel::baseCreate)
    .def("className", &smtk::bridge::cgm::RemoveModel::className)
    .def("classname", &smtk::bridge::cgm::RemoveModel::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::RemoveModel> (*)()) &smtk::bridge::cgm::RemoveModel::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::RemoveModel> (*)(::std::shared_ptr<smtk::bridge::cgm::RemoveModel> &)) &smtk::bridge::cgm::RemoveModel::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::RemoveModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::RemoveModel> (smtk::bridge::cgm::RemoveModel::*)() const) &smtk::bridge::cgm::RemoveModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::RemoveModel> (smtk::bridge::cgm::RemoveModel::*)()) &smtk::bridge::cgm::RemoveModel::shared_from_this)
    ;
  return instance;
}

#endif
