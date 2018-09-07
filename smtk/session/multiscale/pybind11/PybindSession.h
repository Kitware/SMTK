//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_multiscale_Session_h
#define pybind_smtk_session_multiscale_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/multiscale/Session.h"

#include "smtk/model/Session.h"
#include "smtk/session/mesh/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::multiscale::Session, smtk::session::mesh::Session > pybind11_init_smtk_session_multiscale_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::multiscale::Session, smtk::session::mesh::Session > instance(m, "Session");
  instance
    .def("shared_from_this", (std::shared_ptr<smtk::session::multiscale::Session> (smtk::session::multiscale::Session::*)()) &smtk::session::multiscale::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::multiscale::Session> (smtk::session::multiscale::Session::*)() const) &smtk::session::multiscale::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Session> (*)()) &smtk::session::multiscale::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Session> (*)(::std::shared_ptr<smtk::session::multiscale::Session> &)) &smtk::session::multiscale::Session::create, py::arg("ref"))
    .def("name", &smtk::session::multiscale::Session::name)
    .def_static("CastTo", [](const std::shared_ptr<smtk::model::Session> i) {
        return std::dynamic_pointer_cast<smtk::session::multiscale::Session>(i);
      })
    ;
  return instance;
}

#endif
