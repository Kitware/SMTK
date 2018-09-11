//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_Session_h
#define pybind_smtk_session_cgm_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Session, smtk::model::Session > pybind11_init_smtk_session_cgm_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::cgm::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Session> (smtk::session::cgm::Session::*)()) &smtk::session::cgm::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Session> (smtk::session::cgm::Session::*)() const) &smtk::session::cgm::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Session> (*)()) &smtk::session::cgm::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Session> (*)(::std::shared_ptr<smtk::session::cgm::Session> &)) &smtk::session::cgm::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::session::cgm::Session::staticClassName)
    .def("name", &smtk::session::cgm::Session::name)
    .def("registerOperation", &smtk::session::cgm::Session::registerOperation, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperation", &smtk::session::cgm::Session::registerStaticOperation, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperationXML", &smtk::session::cgm::Session::findOperationXML, py::arg("opName"))
    // .def("findOperationConstructor", &smtk::session::cgm::Session::findOperationConstructor, py::arg("opName"))
    .def("inheritsOperations", &smtk::session::cgm::Session::inheritsOperations)
    .def("allSupportedInformation", &smtk::session::cgm::Session::allSupportedInformation)
    .def_static("addManagerEntityToCGM", &smtk::session::cgm::Session::addManagerEntityToCGM, py::arg("ent"))
    .def_static("staticSetup", &smtk::session::cgm::Session::staticSetup, py::arg("optName"), py::arg("optVal"))
    .def("setup", &smtk::session::cgm::Session::setup, py::arg("optName"), py::arg("optVal"))
    .def("maxRelChordErr", &smtk::session::cgm::Session::maxRelChordErr)
    .def("maxAngleErr", &smtk::session::cgm::Session::maxAngleErr)
    ;
  return instance;
}

#endif
