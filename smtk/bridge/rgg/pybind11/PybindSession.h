//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_rgg_Session_h
#define pybind_smtk_bridge_rgg_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/rgg/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::rgg::Session, smtk::model::Session > pybind11_init_smtk_bridge_rgg_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::rgg::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("classname", &smtk::bridge::rgg::Session::classname)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::rgg::Session> (smtk::bridge::rgg::Session::*)()) &smtk::bridge::rgg::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::rgg::Session> (smtk::bridge::rgg::Session::*)() const) &smtk::bridge::rgg::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::rgg::Session> (*)()) &smtk::bridge::rgg::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::rgg::Session> (*)(::std::shared_ptr<smtk::bridge::rgg::Session> &)) &smtk::bridge::rgg::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::bridge::rgg::Session::staticClassName)
    .def("name", &smtk::bridge::rgg::Session::name)
    .def("className", &smtk::bridge::rgg::Session::className)
    .def("registerOperator", &smtk::bridge::rgg::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::rgg::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperatorXML", &smtk::bridge::rgg::Session::findOperatorXML, py::arg("opName"))
    .def("findOperatorConstructor", &smtk::bridge::rgg::Session::findOperatorConstructor, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::rgg::Session::inheritsOperators)
    .def_readwrite_static("sessionName", &smtk::bridge::rgg::Session::sessionName)
    .def_static("CastTo", [](const std::shared_ptr<smtk::model::Session> i) {
        return std::dynamic_pointer_cast<smtk::bridge::rgg::Session>(i);
      })
    ;
  return instance;
}

#endif
