//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_ExtractContours_h
#define pybind_smtk_bridge_polygon_operators_ExtractContours_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/ExtractContours.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::ExtractContours > pybind11_init_smtk_bridge_polygon_ExtractContours(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::ExtractContours > instance(m, "ExtractContours", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::ExtractContours const &>())
    .def("deepcopy", (smtk::bridge::polygon::ExtractContours & (smtk::bridge::polygon::ExtractContours::*)(::smtk::bridge::polygon::ExtractContours const &)) &smtk::bridge::polygon::ExtractContours::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::ExtractContours::ableToOperate)
    .def_static("baseCreate", &smtk::bridge::polygon::ExtractContours::baseCreate)
    .def("className", &smtk::bridge::polygon::ExtractContours::className)
    .def("classname", &smtk::bridge::polygon::ExtractContours::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::ExtractContours> (*)()) &smtk::bridge::polygon::ExtractContours::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::ExtractContours> (*)(::std::shared_ptr<smtk::bridge::polygon::ExtractContours> &)) &smtk::bridge::polygon::ExtractContours::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::ExtractContours::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::ExtractContours> (smtk::bridge::polygon::ExtractContours::*)() const) &smtk::bridge::polygon::ExtractContours::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::ExtractContours> (smtk::bridge::polygon::ExtractContours::*)()) &smtk::bridge::polygon::ExtractContours::shared_from_this)
    ;
  return instance;
}

#endif
