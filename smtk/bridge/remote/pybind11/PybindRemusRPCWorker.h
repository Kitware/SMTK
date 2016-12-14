//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_remote_RemusRPCWorker_h
#define pybind_smtk_bridge_remote_RemusRPCWorker_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/remote/RemusRPCWorker.h"

namespace py = pybind11;

py::class_< smtk::bridge::remote::RemusRPCWorker > pybind11_init_smtk_bridge_remote_RemusRPCWorker(py::module &m)
{
  py::class_< smtk::bridge::remote::RemusRPCWorker > instance(m, "RemusRPCWorker");
  instance
    .def_property("manager", &smtk::bridge::remote::RemusRPCWorker::manager, &smtk::bridge::remote::RemusRPCWorker::setManager)
    .def("classname", &smtk::bridge::remote::RemusRPCWorker::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusRPCWorker> (*)()) &smtk::bridge::remote::RemusRPCWorker::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::remote::RemusRPCWorker> (*)(::std::shared_ptr<smtk::bridge::remote::RemusRPCWorker> &)) &smtk::bridge::remote::RemusRPCWorker::create, py::arg("ref"))
    .def("setOption", &smtk::bridge::remote::RemusRPCWorker::setOption, py::arg("optName"), py::arg("optVal"))
    .def("clearOptions", &smtk::bridge::remote::RemusRPCWorker::clearOptions)
    ;
  return instance;
}

#endif
