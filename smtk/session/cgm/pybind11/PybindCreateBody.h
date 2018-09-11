//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateBody_h
#define pybind_smtk_session_cgm_operators_CreateBody_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateBody.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateBody > pybind11_init_smtk_session_cgm_CreateBody(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateBody > instance(m, "CreateBody", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateBody const &>())
    .def("deepcopy", (smtk::session::cgm::CreateBody & (smtk::session::cgm::CreateBody::*)(::smtk::session::cgm::CreateBody const &)) &smtk::session::cgm::CreateBody::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateBody> (*)()) &smtk::session::cgm::CreateBody::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateBody> (*)(::std::shared_ptr<smtk::session::cgm::CreateBody> &)) &smtk::session::cgm::CreateBody::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateBody::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateBody> (smtk::session::cgm::CreateBody::*)() const) &smtk::session::cgm::CreateBody::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateBody> (smtk::session::cgm::CreateBody::*)()) &smtk::session::cgm::CreateBody::shared_from_this)
    ;
  return instance;
}

#endif
