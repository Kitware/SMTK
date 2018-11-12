//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_Session_h
#define pybind_smtk_session_rgg_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::Session, smtk::model::Session > pybind11_init_smtk_session_rgg_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::session::rgg::Session const &>())
    .def("deepcopy", (smtk::session::rgg::Session & (smtk::session::rgg::Session::*)(::smtk::session::rgg::Session const &)) &smtk::session::rgg::Session::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Session> (*)()) &smtk::session::rgg::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Session> (*)(::std::shared_ptr<smtk::session::rgg::Session> &)) &smtk::session::rgg::Session::create, py::arg("ref"))
    ;
  return instance;
}

#endif
