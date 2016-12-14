//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_Delete_h
#define pybind_smtk_bridge_polygon_operators_Delete_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/Delete.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Delete > pybind11_init_smtk_bridge_polygon_Delete(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::Delete > instance(m, "Delete", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::Delete const &>())
    .def("deepcopy", (smtk::bridge::polygon::Delete & (smtk::bridge::polygon::Delete::*)(::smtk::bridge::polygon::Delete const &)) &smtk::bridge::polygon::Delete::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::Delete::baseCreate)
    .def("className", &smtk::bridge::polygon::Delete::className)
    .def("classname", &smtk::bridge::polygon::Delete::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Delete> (*)()) &smtk::bridge::polygon::Delete::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Delete> (*)(::std::shared_ptr<smtk::bridge::polygon::Delete> &)) &smtk::bridge::polygon::Delete::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::Delete::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::Delete> (smtk::bridge::polygon::Delete::*)() const) &smtk::bridge::polygon::Delete::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::Delete> (smtk::bridge::polygon::Delete::*)()) &smtk::bridge::polygon::Delete::shared_from_this)
    ;
  return instance;
}

#endif
