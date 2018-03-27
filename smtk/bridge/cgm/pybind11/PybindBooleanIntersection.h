//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_BooleanIntersection_h
#define pybind_smtk_bridge_cgm_operators_BooleanIntersection_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/BooleanIntersection.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::BooleanIntersection > pybind11_init_smtk_bridge_cgm_BooleanIntersection(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::BooleanIntersection > instance(m, "BooleanIntersection", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::BooleanIntersection const &>())
    .def("deepcopy", (smtk::bridge::cgm::BooleanIntersection & (smtk::bridge::cgm::BooleanIntersection::*)(::smtk::bridge::cgm::BooleanIntersection const &)) &smtk::bridge::cgm::BooleanIntersection::operator=)
    .def("ableToOperate", &smtk::bridge::cgm::BooleanIntersection::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanIntersection> (*)()) &smtk::bridge::cgm::BooleanIntersection::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanIntersection> (*)(::std::shared_ptr<smtk::bridge::cgm::BooleanIntersection> &)) &smtk::bridge::cgm::BooleanIntersection::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::BooleanIntersection::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::BooleanIntersection> (smtk::bridge::cgm::BooleanIntersection::*)() const) &smtk::bridge::cgm::BooleanIntersection::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::BooleanIntersection> (smtk::bridge::cgm::BooleanIntersection::*)()) &smtk::bridge::cgm::BooleanIntersection::shared_from_this)
    ;
  return instance;
}

#endif
