//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_Session_h
#define pybind_smtk_bridge_multiscale_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/Session.h"

#include "smtk/bridge/mesh/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > pybind11_init_smtk_bridge_multiscale_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > instance(m, "Session");
  instance
    .def("classname", &smtk::bridge::multiscale::Session::classname)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::Session> (smtk::bridge::multiscale::Session::*)()) &smtk::bridge::multiscale::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::Session> (smtk::bridge::multiscale::Session::*)() const) &smtk::bridge::multiscale::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Session> (*)()) &smtk::bridge::multiscale::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Session> (*)(::std::shared_ptr<smtk::bridge::multiscale::Session> &)) &smtk::bridge::multiscale::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::bridge::multiscale::Session::staticClassName)
    .def("name", &smtk::bridge::multiscale::Session::name)
    .def("className", &smtk::bridge::multiscale::Session::className)
    .def("registerOperator", &smtk::bridge::multiscale::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::multiscale::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("findOperatorXML", &smtk::bridge::multiscale::Session::findOperatorXML, py::arg("opName"))
    .def("findOperatorConstructor", &smtk::bridge::multiscale::Session::findOperatorConstructor, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::multiscale::Session::inheritsOperators)
    .def_readwrite_static("sessionName", &smtk::bridge::multiscale::Session::sessionName)
    ;
  return instance;
}

#endif
