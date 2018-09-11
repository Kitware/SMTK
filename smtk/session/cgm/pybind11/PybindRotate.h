//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Rotate_h
#define pybind_smtk_session_cgm_operators_Rotate_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Rotate.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Rotate > pybind11_init_smtk_session_cgm_Rotate(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Rotate > instance(m, "Rotate", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Rotate const &>())
    .def("deepcopy", (smtk::session::cgm::Rotate & (smtk::session::cgm::Rotate::*)(::smtk::session::cgm::Rotate const &)) &smtk::session::cgm::Rotate::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Rotate> (*)()) &smtk::session::cgm::Rotate::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Rotate> (*)(::std::shared_ptr<smtk::session::cgm::Rotate> &)) &smtk::session::cgm::Rotate::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Rotate::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Rotate> (smtk::session::cgm::Rotate::*)() const) &smtk::session::cgm::Rotate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Rotate> (smtk::session::cgm::Rotate::*)()) &smtk::session::cgm::Rotate::shared_from_this)
    ;
  return instance;
}

#endif
