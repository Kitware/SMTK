//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_BooleanUnion_h
#define pybind_smtk_session_cgm_operators_BooleanUnion_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/BooleanUnion.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::BooleanUnion > pybind11_init_smtk_session_cgm_BooleanUnion(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::BooleanUnion > instance(m, "BooleanUnion", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::BooleanUnion const &>())
    .def("deepcopy", (smtk::session::cgm::BooleanUnion & (smtk::session::cgm::BooleanUnion::*)(::smtk::session::cgm::BooleanUnion const &)) &smtk::session::cgm::BooleanUnion::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanUnion> (*)()) &smtk::session::cgm::BooleanUnion::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanUnion> (*)(::std::shared_ptr<smtk::session::cgm::BooleanUnion> &)) &smtk::session::cgm::BooleanUnion::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::BooleanUnion::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::BooleanUnion> (smtk::session::cgm::BooleanUnion::*)() const) &smtk::session::cgm::BooleanUnion::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::BooleanUnion> (smtk::session::cgm::BooleanUnion::*)()) &smtk::session::cgm::BooleanUnion::shared_from_this)
    ;
  return instance;
}

#endif
