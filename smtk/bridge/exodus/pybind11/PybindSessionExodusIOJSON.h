//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_exodus_SessionExodusIOJSON_h
#define pybind_smtk_bridge_exodus_SessionExodusIOJSON_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/exodus/SessionExodusIOJSON.h"

#include "cJSON.h"

namespace py = pybind11;

py::class_< smtk::bridge::exodus::SessionIOJSON, smtk::model::SessionIOJSON > pybind11_init_smtk_bridge_exodus_SessionIOJSON(py::module &m)
{
  py::class_< smtk::bridge::exodus::SessionIOJSON, smtk::model::SessionIOJSON > instance(m, "SessionIOJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::exodus::SessionIOJSON const &>())
    .def("deepcopy", (smtk::bridge::exodus::SessionIOJSON & (smtk::bridge::exodus::SessionIOJSON::*)(::smtk::bridge::exodus::SessionIOJSON const &)) &smtk::bridge::exodus::SessionIOJSON::operator=)
    .def("classname", &smtk::bridge::exodus::SessionIOJSON::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::SessionIOJSON> (*)()) &smtk::bridge::exodus::SessionIOJSON::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::SessionIOJSON> (*)(::std::shared_ptr<smtk::bridge::exodus::SessionIOJSON> &)) &smtk::bridge::exodus::SessionIOJSON::create, py::arg("ref"))
    .def("importJSON", &smtk::bridge::exodus::SessionIOJSON::importJSON, py::arg("modelMgr"), py::arg("session"), py::arg("sessionRec"), py::arg("loadNativeModels") = false)
    .def("exportJSON", (int (smtk::bridge::exodus::SessionIOJSON::*)(::smtk::model::ManagerPtr, ::smtk::model::SessionPtr const &, ::cJSON *, bool)) &smtk::bridge::exodus::SessionIOJSON::exportJSON, py::arg("modelMgr"), py::arg("sessPtr"), py::arg("sessionRec"), py::arg("writeNativeModels") = false)
    .def("exportJSON", (int (smtk::bridge::exodus::SessionIOJSON::*)(::smtk::model::ManagerPtr, ::smtk::model::SessionPtr const &, ::smtk::common::UUIDs const &, ::cJSON *, bool)) &smtk::bridge::exodus::SessionIOJSON::exportJSON, py::arg("modelMgr"), py::arg("session"), py::arg("modelIds"), py::arg("sessionRec"), py::arg("writeNativeModels") = false)
    ;
  return instance;
}

#endif
