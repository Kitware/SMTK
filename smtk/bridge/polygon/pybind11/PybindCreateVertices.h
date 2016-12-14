//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateVertices_h
#define pybind_smtk_bridge_polygon_operators_CreateVertices_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateVertices.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateVertices > pybind11_init_smtk_bridge_polygon_CreateVertices(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateVertices > instance(m, "CreateVertices", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateVertices const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateVertices & (smtk::bridge::polygon::CreateVertices::*)(::smtk::bridge::polygon::CreateVertices const &)) &smtk::bridge::polygon::CreateVertices::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateVertices::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateVertices::className)
    .def("classname", &smtk::bridge::polygon::CreateVertices::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateVertices> (*)()) &smtk::bridge::polygon::CreateVertices::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateVertices> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateVertices> &)) &smtk::bridge::polygon::CreateVertices::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateVertices::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateVertices> (smtk::bridge::polygon::CreateVertices::*)() const) &smtk::bridge::polygon::CreateVertices::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateVertices> (smtk::bridge::polygon::CreateVertices::*)()) &smtk::bridge::polygon::CreateVertices::shared_from_this)
    ;
  return instance;
}

#endif
