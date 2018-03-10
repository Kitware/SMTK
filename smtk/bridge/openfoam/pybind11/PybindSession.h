//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_openfoam_Session_h
#define pybind_smtk_bridge_openfoam_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/openfoam/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::openfoam::Session, smtk::model::Session > pybind11_init_smtk_bridge_openfoam_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::openfoam::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("classname", &smtk::bridge::openfoam::Session::classname)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::openfoam::Session> (smtk::bridge::openfoam::Session::*)()) &smtk::bridge::openfoam::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::openfoam::Session> (smtk::bridge::openfoam::Session::*)() const) &smtk::bridge::openfoam::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::openfoam::Session> (*)()) &smtk::bridge::openfoam::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::openfoam::Session> (*)(::std::shared_ptr<smtk::bridge::openfoam::Session> &)) &smtk::bridge::openfoam::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::bridge::openfoam::Session::staticClassName)
    .def("name", &smtk::bridge::openfoam::Session::name)
    .def("className", &smtk::bridge::openfoam::Session::className)
    .def("registerOperator", &smtk::bridge::openfoam::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::openfoam::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperatorXML", &smtk::bridge::openfoam::Session::findOperatorXML, py::arg("opName"))
    .def("findOperatorConstructor", &smtk::bridge::openfoam::Session::findOperatorConstructor, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::openfoam::Session::inheritsOperators)
    .def("workingDirectory", &smtk::bridge::openfoam::Session::workingDirectory)
    .def("setWorkingDirectory", &smtk::bridge::openfoam::Session::setWorkingDirectory)
    .def("createWorkingDirectory", &smtk::bridge::openfoam::Session::createWorkingDirectory)
    .def("removeWorkingDirectory", &smtk::bridge::openfoam::Session::removeWorkingDirectory)
    .def("workingDirectoryExists", &smtk::bridge::openfoam::Session::workingDirectoryExists)
    .def_readwrite_static("sessionName", &smtk::bridge::openfoam::Session::sessionName)
    .def_static("CastTo", [](const std::shared_ptr<smtk::model::Session> i) {
        return std::dynamic_pointer_cast<smtk::bridge::openfoam::Session>(i);
      })
    ;
  return instance;
}

#endif
