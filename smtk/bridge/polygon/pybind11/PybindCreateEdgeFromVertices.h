//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateEdgeFromVertices_h
#define pybind_smtk_bridge_polygon_operators_CreateEdgeFromVertices_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateEdgeFromVertices.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromVertices > pybind11_init_smtk_bridge_polygon_CreateEdgeFromVertices(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromVertices > instance(m, "CreateEdgeFromVertices", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateEdgeFromVertices const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateEdgeFromVertices & (smtk::bridge::polygon::CreateEdgeFromVertices::*)(::smtk::bridge::polygon::CreateEdgeFromVertices const &)) &smtk::bridge::polygon::CreateEdgeFromVertices::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateEdgeFromVertices::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateEdgeFromVertices::className)
    .def("classname", &smtk::bridge::polygon::CreateEdgeFromVertices::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromVertices> (*)()) &smtk::bridge::polygon::CreateEdgeFromVertices::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromVertices> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromVertices> &)) &smtk::bridge::polygon::CreateEdgeFromVertices::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateEdgeFromVertices::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateEdgeFromVertices> (smtk::bridge::polygon::CreateEdgeFromVertices::*)() const) &smtk::bridge::polygon::CreateEdgeFromVertices::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromVertices> (smtk::bridge::polygon::CreateEdgeFromVertices::*)()) &smtk::bridge::polygon::CreateEdgeFromVertices::shared_from_this)
    ;
  return instance;
}

#endif
