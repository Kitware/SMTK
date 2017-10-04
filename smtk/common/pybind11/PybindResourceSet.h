//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Set_h
#define pybind_smtk_common_Set_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/common/Set.h"

#include "smtk/common/Resource.h"

namespace py = pybind11;

py::class_< smtk::common::Set > pybind11_init_smtk_common_Set(py::module &m)
{
  py::class_< smtk::common::Set > instance(m, "Set");
  py::enum_<smtk::common::Set::Role>(instance, "Role")
    .value("NOT_DEFINED", smtk::common::Set::Role::NOT_DEFINED)
    .value("TEMPLATE", smtk::common::Set::Role::TEMPLATE)
    .value("SCENARIO", smtk::common::Set::Role::SCENARIO)
    .value("INSTANCE", smtk::common::Set::Role::INSTANCE)
    .export_values();
  py::enum_<smtk::common::Set::State>(instance, "ResourceState")
    .value("NOT_LOADED", smtk::common::Set::State::NOT_LOADED)
    .value("LOADED", smtk::common::Set::State::LOADED)
    .value("LOAD_ERROR", smtk::common::Set::State::LOAD_ERROR)
    .export_values();
  instance
    .def(py::init<::smtk::common::Set const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::common::Set & (smtk::common::Set::*)(::smtk::common::ResourceSet const &)) &smtk::common::ResourceSet::operator=)
    .def("add", &smtk::common::Set::addResource, py::arg("resource"), py::arg("id"), py::arg("link") = "", py::arg("arg3") = ::smtk::common::Set::Role::NOT_DEFINED)
    .def("addInfo", &smtk::common::Set::addInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link") = "")
    .def("get", &smtk::common::Set::get, py::arg("id"), py::arg("resource"))
    .def("get", [](const smtk::common::Set& set, std::string id){ smtk::common::ResourcePtr resource; set.get(id, resource); return resource; })
    .def("linkStartPath", &smtk::common::Set::linkStartPath)
    .def("numberOfResources", &smtk::common::Set::numberOfResources)
    .def("resourceIds", &smtk::common::Set::resourceIds)
    .def("resourceInfo", &smtk::common::Set::resourceInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link"))
    .def_static("role2String", &smtk::common::Set::role2String, py::arg("role"))
    .def("setLinkStartPath", &smtk::common::Set::setLinkStartPath, py::arg("path"))
    .def_static("state2String", &smtk::common::Set::state2String, py::arg("state"))
    .def_static("string2Role", &smtk::common::Set::string2Role, py::arg("s"))
    ;
  return instance;
}

#endif
