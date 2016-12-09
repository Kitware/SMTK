//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_ResourceSet_h
#define pybind_smtk_common_ResourceSet_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/common/ResourceSet.h"

#include "smtk/common/Resource.h"

namespace py = pybind11;

py::class_< smtk::common::ResourceSet > pybind11_init_smtk_common_ResourceSet(py::module &m)
{
  py::class_< smtk::common::ResourceSet > instance(m, "ResourceSet");
  py::enum_<smtk::common::ResourceSet::ResourceRole>(instance, "ResourceRole")
    .value("NOT_DEFINED", smtk::common::ResourceSet::ResourceRole::NOT_DEFINED)
    .value("TEMPLATE", smtk::common::ResourceSet::ResourceRole::TEMPLATE)
    .value("SCENARIO", smtk::common::ResourceSet::ResourceRole::SCENARIO)
    .value("INSTANCE", smtk::common::ResourceSet::ResourceRole::INSTANCE)
    .export_values();
  py::enum_<smtk::common::ResourceSet::ResourceState>(instance, "ResourceState")
    .value("NOT_LOADED", smtk::common::ResourceSet::ResourceState::NOT_LOADED)
    .value("LOADED", smtk::common::ResourceSet::ResourceState::LOADED)
    .value("LOAD_ERROR", smtk::common::ResourceSet::ResourceState::LOAD_ERROR)
    .export_values();
  instance
    .def(py::init<::smtk::common::ResourceSet const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::common::ResourceSet & (smtk::common::ResourceSet::*)(::smtk::common::ResourceSet const &)) &smtk::common::ResourceSet::operator=)
    .def("addResource", &smtk::common::ResourceSet::addResource, py::arg("resource"), py::arg("id"), py::arg("link") = "", py::arg("arg3") = ::smtk::common::ResourceSet::ResourceRole::NOT_DEFINED)
    .def("addResourceInfo", &smtk::common::ResourceSet::addResourceInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link") = "")
    .def("get", &smtk::common::ResourceSet::get, py::arg("id"), py::arg("resource"))
    .def("get", [](const smtk::common::ResourceSet& set, std::string id){ smtk::common::ResourcePtr resource; set.get(id, resource); return resource; })
    .def("linkStartPath", &smtk::common::ResourceSet::linkStartPath)
    .def("numberOfResources", &smtk::common::ResourceSet::numberOfResources)
    .def("resourceIds", &smtk::common::ResourceSet::resourceIds)
    .def("resourceInfo", &smtk::common::ResourceSet::resourceInfo, py::arg("id"), py::arg("type"), py::arg("role"), py::arg("state"), py::arg("link"))
    .def_static("role2String", &smtk::common::ResourceSet::role2String, py::arg("role"))
    .def("setLinkStartPath", &smtk::common::ResourceSet::setLinkStartPath, py::arg("path"))
    .def_static("state2String", &smtk::common::ResourceSet::state2String, py::arg("state"))
    .def_static("string2Role", &smtk::common::ResourceSet::string2Role, py::arg("s"))
    ;
  return instance;
}

#endif
