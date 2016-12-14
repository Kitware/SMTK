//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_remote_RemusConnections_h
#define pybind_smtk_bridge_remote_RemusConnections_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/remote/RemusConnections.h"

namespace py = pybind11;

py::class_< smtk::bridge::remote::RemusConnections > pybind11_init_smtk_bridge_remote_RemusConnections(py::module &m)
{
  py::class_< smtk::bridge::remote::RemusConnections > instance(m, "RemusConnections");
  instance
    .def(py::init<::smtk::bridge::remote::RemusConnections const &>())
    .def("deepcopy", (smtk::bridge::remote::RemusConnections & (smtk::bridge::remote::RemusConnections::*)(::smtk::bridge::remote::RemusConnections const &)) &smtk::bridge::remote::RemusConnections::operator=)
    .def("classname", &smtk::bridge::remote::RemusConnections::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusConnections> (*)()) &smtk::bridge::remote::RemusConnections::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusConnections> (*)(::std::shared_ptr<smtk::bridge::remote::RemusConnections> &)) &smtk::bridge::remote::RemusConnections::create, py::arg("ref"))
    .def("connectToServer", &smtk::bridge::remote::RemusConnections::connectToServer, py::arg("host") = std::string(), py::arg("port") = -1)
    ;
  return instance;
}

#endif
