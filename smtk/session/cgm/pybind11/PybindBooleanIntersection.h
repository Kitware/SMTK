//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_BooleanIntersection_h
#define pybind_smtk_session_cgm_operators_BooleanIntersection_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/BooleanIntersection.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::BooleanIntersection > pybind11_init_smtk_session_cgm_BooleanIntersection(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::BooleanIntersection > instance(m, "BooleanIntersection", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::BooleanIntersection const &>())
    .def("deepcopy", (smtk::session::cgm::BooleanIntersection & (smtk::session::cgm::BooleanIntersection::*)(::smtk::session::cgm::BooleanIntersection const &)) &smtk::session::cgm::BooleanIntersection::operator=)
    .def("ableToOperate", &smtk::session::cgm::BooleanIntersection::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanIntersection> (*)()) &smtk::session::cgm::BooleanIntersection::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanIntersection> (*)(::std::shared_ptr<smtk::session::cgm::BooleanIntersection> &)) &smtk::session::cgm::BooleanIntersection::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::BooleanIntersection::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::BooleanIntersection> (smtk::session::cgm::BooleanIntersection::*)() const) &smtk::session::cgm::BooleanIntersection::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::BooleanIntersection> (smtk::session::cgm::BooleanIntersection::*)()) &smtk::session::cgm::BooleanIntersection::shared_from_this)
    ;
  return instance;
}

#endif
