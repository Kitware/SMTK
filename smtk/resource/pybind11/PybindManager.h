//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Manager_h
#define pybind_smtk_resource_Manager_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <vector>

namespace py = pybind11;

PySharedPtrClass<smtk::resource::Manager> pybind11_init_smtk_resource_Manager(py::module& m)
{
  PySharedPtrClass<smtk::resource::Manager> instance(m, "Manager");
  instance
    .def("add", (bool (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::add)
    .def("add", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::add)
    .def_static("create", (std::shared_ptr<smtk::resource::Manager> (*)()) &smtk::resource::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::resource::Manager> (*)(std::shared_ptr<smtk::resource::Manager>&)) &smtk::resource::Manager::create)
    .def("createResource", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&)) &smtk::resource::Manager::create)
    .def("createResource", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&)) &smtk::resource::Manager::create)
    .def("createResource", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&, const smtk::common::UUID&)) &smtk::resource::Manager::create)
    .def("createResource", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, const smtk::common::UUID&)) &smtk::resource::Manager::create)
    .def("get", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::common::UUID&)) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<const smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::common::UUID&) const) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&)) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<const smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&) const) &smtk::resource::Manager::get)
    .def("find", (std::set<std::shared_ptr<smtk::resource::Resource>> (smtk::resource::Manager::*)(const std::string&)) &smtk::resource::Manager::find)
    .def("find", (std::set<std::shared_ptr<smtk::resource::Resource>> (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&)) &smtk::resource::Manager::find)
    .def("metadata", [](smtk::resource::Manager& man) { std::vector<std::reference_wrapper<smtk::resource::Metadata>> vec; vec.reserve(man.metadata().size()); for (auto md : man.metadata()) { vec.push_back(md); } return vec; })
    .def("observers", (smtk::resource::Observers & (smtk::resource::Manager::*)()) &smtk::resource::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("observers", (smtk::resource::Observers const & (smtk::resource::Manager::*)() const) &smtk::resource::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("read", (smtk::resource::ResourcePtr (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, const std::string&)) &smtk::resource::Manager::read)
    .def("read", (smtk::resource::ResourcePtr (smtk::resource::Manager::*)(const std::string&, const std::string&)) &smtk::resource::Manager::read)
    .def("remove", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::remove)
    .def("resources", [](smtk::resource::Manager& man) { return std::vector<std::shared_ptr<smtk::resource::Resource>>(man.resources().begin(), man.resources().end()); })
    .def("write", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::write)
    .def("write", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&, const std::string&)) &smtk::resource::Manager::write)
    ;
  return instance;
}

#endif
