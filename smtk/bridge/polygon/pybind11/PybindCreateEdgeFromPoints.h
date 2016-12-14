//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateEdgeFromPoints_h
#define pybind_smtk_bridge_polygon_operators_CreateEdgeFromPoints_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateEdgeFromPoints.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromPoints > pybind11_init_smtk_bridge_polygon_CreateEdgeFromPoints(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromPoints > instance(m, "CreateEdgeFromPoints", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateEdgeFromPoints const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateEdgeFromPoints & (smtk::bridge::polygon::CreateEdgeFromPoints::*)(::smtk::bridge::polygon::CreateEdgeFromPoints const &)) &smtk::bridge::polygon::CreateEdgeFromPoints::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateEdgeFromPoints::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateEdgeFromPoints::className)
    .def("classname", &smtk::bridge::polygon::CreateEdgeFromPoints::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromPoints> (*)()) &smtk::bridge::polygon::CreateEdgeFromPoints::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromPoints> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromPoints> &)) &smtk::bridge::polygon::CreateEdgeFromPoints::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateEdgeFromPoints::name)
    .def("process", &smtk::bridge::polygon::CreateEdgeFromPoints::process, py::arg("pnts"), py::arg("numCoordsPerPoint"), py::arg("parentModel"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateEdgeFromPoints> (smtk::bridge::polygon::CreateEdgeFromPoints::*)() const) &smtk::bridge::polygon::CreateEdgeFromPoints::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateEdgeFromPoints> (smtk::bridge::polygon::CreateEdgeFromPoints::*)()) &smtk::bridge::polygon::CreateEdgeFromPoints::shared_from_this)
    ;
  return instance;
}

#endif
