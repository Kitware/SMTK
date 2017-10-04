//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Set_h
#define pybind_smtk_resource_Set_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/resource/Set.h"

#include "smtk/resource/Resource.h"

namespace py = pybind11;

py::class_< smtk::resource::Set > pybind11_init_smtk_resource_Set(py::module &m)
{
  py::class_< smtk::resource::Set > instance(m, "Set");
  py::enum_<smtk::resource::Set::Role>(instance, "Role")
    .value("NOT_DEFINED", smtk::resource::Set::Role::NOT_DEFINED)
    .value("TEMPLATE", smtk::resource::Set::Role::TEMPLATE)
    .value("SCENARIO", smtk::resource::Set::Role::SCENARIO)
    .value("INSTANCE", smtk::resource::Set::Role::INSTANCE)
    .export_values();
  py::enum_<smtk::resource::Set::State>(instance, "State")
    .value("NOT_LOADED", smtk::resource::Set::State::NOT_LOADED)
    .value("LOADED", smtk::resource::Set::State::LOADED)
    .value("LOAD_ERROR", smtk::resource::Set::State::LOAD_ERROR)
    .export_values();
  instance
    .def(py::init<::smtk::resource::Set const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::resource::Set & (smtk::resource::Set::*)(::smtk::resource::Set const &)) &smtk::resource::Set::operator=)
    .def("add", &smtk::resource::Set::add, py::arg("resource"), py::arg("id"), py::arg("link") = "", py::arg("arg3") = ::smtk::resource::Set::Role::NOT_DEFINED)
    .def("addInfo", &smtk::resource::Set::addInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link") = "")
    .def("get", &smtk::resource::Set::get, py::arg("id"), py::arg("resource"))
    .def("get", [](const smtk::resource::Set& set, std::string id){ smtk::resource::ResourcePtr resource; set.get(id, resource); return resource; })
    .def("linkStartPath", &smtk::resource::Set::linkStartPath)
    .def("numberOfResources", &smtk::resource::Set::numberOfResources)
    .def("resourceIds", &smtk::resource::Set::resourceIds)
    .def("resourceInfo", &smtk::resource::Set::resourceInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link"))
    .def_static("role2String", &smtk::resource::Set::role2String, py::arg("role"))
    .def("setLinkStartPath", &smtk::resource::Set::setLinkStartPath, py::arg("path"))
    .def_static("state2String", &smtk::resource::Set::state2String, py::arg("state"))
    .def_static("string2Role", &smtk::resource::Set::string2Role, py::arg("s"))
    ;
  return instance;
}

#endif
