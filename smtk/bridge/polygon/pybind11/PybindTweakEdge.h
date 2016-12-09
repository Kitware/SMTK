//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_TweakEdge_h
#define pybind_smtk_bridge_polygon_operators_TweakEdge_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/TweakEdge.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::TweakEdge > pybind11_init_smtk_bridge_polygon_TweakEdge(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::TweakEdge > instance(m, "TweakEdge", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::TweakEdge const &>())
    .def("deepcopy", (smtk::bridge::polygon::TweakEdge & (smtk::bridge::polygon::TweakEdge::*)(::smtk::bridge::polygon::TweakEdge const &)) &smtk::bridge::polygon::TweakEdge::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::TweakEdge::baseCreate)
    .def("className", &smtk::bridge::polygon::TweakEdge::className)
    .def("classname", &smtk::bridge::polygon::TweakEdge::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::TweakEdge> (*)()) &smtk::bridge::polygon::TweakEdge::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::TweakEdge> (*)(::std::shared_ptr<smtk::bridge::polygon::TweakEdge> &)) &smtk::bridge::polygon::TweakEdge::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::TweakEdge::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::TweakEdge> (smtk::bridge::polygon::TweakEdge::*)() const) &smtk::bridge::polygon::TweakEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::TweakEdge> (smtk::bridge::polygon::TweakEdge::*)()) &smtk::bridge::polygon::TweakEdge::shared_from_this)
    ;
  return instance;
}

#endif
