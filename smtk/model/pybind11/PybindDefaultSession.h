//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_DefaultSession_h
#define pybind_smtk_model_DefaultSession_h

#include <pybind11/pybind11.h>

#include "smtk/model/DefaultSession.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::DefaultSession, smtk::model::Session > pybind11_init_smtk_model_DefaultSession(py::module &m)
{
  PySharedPtrClass< smtk::model::DefaultSession, smtk::model::Session > instance(m, "DefaultSession");
  instance
    .def(py::init<::smtk::model::DefaultSession const &>())
    .def("deepcopy", (smtk::model::DefaultSession & (smtk::model::DefaultSession::*)(::smtk::model::DefaultSession const &)) &smtk::model::DefaultSession::operator=)
    .def("backsRemoteSession", &smtk::model::DefaultSession::backsRemoteSession, py::arg("remoteSessionName"), py::arg("sessionId"))
    .def_static("create", (std::shared_ptr<smtk::model::DefaultSession> (*)()) &smtk::model::DefaultSession::create)
    .def_static("create", (std::shared_ptr<smtk::model::DefaultSession> (*)(::std::shared_ptr<smtk::model::DefaultSession> &)) &smtk::model::DefaultSession::create, py::arg("ref"))
    .def("name", &smtk::model::DefaultSession::name)
    .def("remoteName", &smtk::model::DefaultSession::remoteName)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::DefaultSession> (smtk::model::DefaultSession::*)() const) &smtk::model::DefaultSession::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::DefaultSession> (smtk::model::DefaultSession::*)()) &smtk::model::DefaultSession::shared_from_this)
    .def_static("staticClassName", &smtk::model::DefaultSession::staticClassName)
    ;
  return instance;
}

#endif
