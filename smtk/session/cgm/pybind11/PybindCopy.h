//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Copy_h
#define pybind_smtk_session_cgm_operators_Copy_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Copy.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Copy > pybind11_init_smtk_session_cgm_Copy(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Copy > instance(m, "Copy", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Copy const &>())
    .def("deepcopy", (smtk::session::cgm::Copy & (smtk::session::cgm::Copy::*)(::smtk::session::cgm::Copy const &)) &smtk::session::cgm::Copy::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Copy> (*)()) &smtk::session::cgm::Copy::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Copy> (*)(::std::shared_ptr<smtk::session::cgm::Copy> &)) &smtk::session::cgm::Copy::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Copy::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Copy> (smtk::session::cgm::Copy::*)() const) &smtk::session::cgm::Copy::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Copy> (smtk::session::cgm::Copy::*)()) &smtk::session::cgm::Copy::shared_from_this)
    ;
  return instance;
}

#endif
