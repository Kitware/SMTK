//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_remote_Session_h
#define pybind_smtk_bridge_remote_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/remote/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::remote::Session, smtk::model::Session > pybind11_init_smtk_bridge_remote_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::remote::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::bridge::remote::Session const &>())
    .def("deepcopy", (smtk::bridge::remote::Session & (smtk::bridge::remote::Session::*)(::smtk::bridge::remote::Session const &)) &smtk::bridge::remote::Session::operator=)
    .def("classname", &smtk::bridge::remote::Session::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::Session> (*)()) &smtk::bridge::remote::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::Session> (*)(::std::shared_ptr<smtk::bridge::remote::Session> &)) &smtk::bridge::remote::Session::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::remote::Session> (smtk::bridge::remote::Session::*)()) &smtk::bridge::remote::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::remote::Session> (smtk::bridge::remote::Session::*)() const) &smtk::bridge::remote::Session::shared_from_this)
    .def_static("staticClassName", &smtk::bridge::remote::Session::staticClassName)
    .def("name", &smtk::bridge::remote::Session::name)
    .def("className", &smtk::bridge::remote::Session::className)
    .def("registerOperator", &smtk::bridge::remote::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::remote::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperatorXML", &smtk::bridge::remote::Session::findOperatorXML, py::arg("opName"))
    // .def("findOperatorConstructor", &smtk::bridge::remote::Session::findOperatorConstructor, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::remote::Session::inheritsOperators)
    ;
  return instance;
}

#endif
