//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CleanGeometry_h
#define pybind_smtk_bridge_polygon_operators_CleanGeometry_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CleanGeometry.h"

#include "smtk/bridge/polygon/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CleanGeometry, smtk::bridge::polygon::Operator > pybind11_init_smtk_bridge_polygon_CleanGeometry(py::module &m)
{
  PySharedPtrClass< smtk::bridge::polygon::CleanGeometry, smtk::bridge::polygon::Operator > instance(m, "CleanGeometry");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CleanGeometry const &>())
    .def("deepcopy", (smtk::bridge::polygon::CleanGeometry & (smtk::bridge::polygon::CleanGeometry::*)(::smtk::bridge::polygon::CleanGeometry const &)) &smtk::bridge::polygon::CleanGeometry::operator=)
    .def("classname", &smtk::bridge::polygon::CleanGeometry::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CleanGeometry> (*)()) &smtk::bridge::polygon::CleanGeometry::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CleanGeometry> (*)(::std::shared_ptr<smtk::bridge::polygon::CleanGeometry> &)) &smtk::bridge::polygon::CleanGeometry::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CleanGeometry> (smtk::bridge::polygon::CleanGeometry::*)() const) &smtk::bridge::polygon::CleanGeometry::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CleanGeometry> (smtk::bridge::polygon::CleanGeometry::*)()) &smtk::bridge::polygon::CleanGeometry::shared_from_this)
    ;
  return instance;
}

#endif
