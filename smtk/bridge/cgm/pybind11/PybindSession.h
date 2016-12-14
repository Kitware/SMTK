//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_Session_h
#define pybind_smtk_bridge_cgm_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Session, smtk::model::Session > pybind11_init_smtk_bridge_cgm_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::cgm::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("classname", &smtk::bridge::cgm::Session::classname)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Session> (smtk::bridge::cgm::Session::*)()) &smtk::bridge::cgm::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Session> (smtk::bridge::cgm::Session::*)() const) &smtk::bridge::cgm::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Session> (*)()) &smtk::bridge::cgm::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Session> (*)(::std::shared_ptr<smtk::bridge::cgm::Session> &)) &smtk::bridge::cgm::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::bridge::cgm::Session::staticClassName)
    .def("name", &smtk::bridge::cgm::Session::name)
    .def("className", &smtk::bridge::cgm::Session::className)
    .def("registerOperator", &smtk::bridge::cgm::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::cgm::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperatorXML", &smtk::bridge::cgm::Session::findOperatorXML, py::arg("opName"))
    // .def("findOperatorConstructor", &smtk::bridge::cgm::Session::findOperatorConstructor, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::cgm::Session::inheritsOperators)
    .def("allSupportedInformation", &smtk::bridge::cgm::Session::allSupportedInformation)
    .def_static("addManagerEntityToCGM", &smtk::bridge::cgm::Session::addManagerEntityToCGM, py::arg("ent"))
    .def_static("staticSetup", &smtk::bridge::cgm::Session::staticSetup, py::arg("optName"), py::arg("optVal"))
    .def("setup", &smtk::bridge::cgm::Session::setup, py::arg("optName"), py::arg("optVal"))
    .def("maxRelChordErr", &smtk::bridge::cgm::Session::maxRelChordErr)
    .def("maxAngleErr", &smtk::bridge::cgm::Session::maxAngleErr)
    ;
  return instance;
}

#endif
