//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_Session_h
#define pybind_smtk_session_polygon_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::polygon::Session, smtk::model::Session > pybind11_init_smtk_session_polygon_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::polygon::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::Session> (smtk::session::polygon::Session::*)()) &smtk::session::polygon::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::Session> (smtk::session::polygon::Session::*)() const) &smtk::session::polygon::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Session> (*)()) &smtk::session::polygon::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Session> (*)(::std::shared_ptr<smtk::session::polygon::Session> &)) &smtk::session::polygon::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::session::polygon::Session::staticClassName)
    .def("name", &smtk::session::polygon::Session::name)
    .def("allSupportedInformation", &smtk::session::polygon::Session::allSupportedInformation)
    ;
  return instance;
}

#endif
