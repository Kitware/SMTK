//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateModel_h
#define pybind_smtk_bridge_polygon_operators_CreateModel_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateModel.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateModel > pybind11_init_smtk_bridge_polygon_CreateModel(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateModel > instance(m, "CreateModel", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateModel const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateModel & (smtk::bridge::polygon::CreateModel::*)(::smtk::bridge::polygon::CreateModel const &)) &smtk::bridge::polygon::CreateModel::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateModel::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateModel::className)
    .def("classname", &smtk::bridge::polygon::CreateModel::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateModel> (*)()) &smtk::bridge::polygon::CreateModel::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateModel> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateModel> &)) &smtk::bridge::polygon::CreateModel::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateModel> (smtk::bridge::polygon::CreateModel::*)() const) &smtk::bridge::polygon::CreateModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateModel> (smtk::bridge::polygon::CreateModel::*)()) &smtk::bridge::polygon::CreateModel::shared_from_this)
    ;
  return instance;
}

#endif
