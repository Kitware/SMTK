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

inline PySharedPtrClass<smtk::resource::Manager> pybind11_init_smtk_resource_Manager(py::module& m)
{
  PySharedPtrClass<smtk::resource::Manager> instance(m, "Manager");
  instance
    .def("add", (bool (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::add)
    .def("add", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::add)
    .def_static("create", (std::shared_ptr<smtk::resource::Manager> (*)()) &smtk::resource::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::resource::Manager> (*)(std::shared_ptr<smtk::resource::Manager>&)) &smtk::resource::Manager::create)
    .def("createResource", [](smtk::resource::Manager& manager, const std::string& resourceType)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(resourceType, empty);
      }, py::arg("resourceType")
    )
    .def("createResource", [](smtk::resource::Manager& manager, const smtk::resource::Resource::Index& resourceTypeIndex)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(resourceTypeIndex, empty);
      }, py::arg("resourceTypeIndex")
    )
    .def("createResource", [](smtk::resource::Manager& manager, const std::string& resourceType, const smtk::common::UUID& uid)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(resourceType, uid, empty);
      }, py::arg("resourceType"), py::arg("uuid")
    )
    .def("createResource", [](smtk::resource::Manager& manager, const smtk::resource::Resource::Index& resourceTypeIndex, const smtk::common::UUID& uid)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(resourceTypeIndex, uid, empty);
      }, py::arg("resourceTypeIndex"), py::arg("uuid")
    )
    .def("get", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::common::UUID&)) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<const smtk::resource::Resource> (smtk::resource::Manager::*)(const smtk::common::UUID&) const) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&)) &smtk::resource::Manager::get)
    .def("get", (std::shared_ptr<const smtk::resource::Resource> (smtk::resource::Manager::*)(const std::string&) const) &smtk::resource::Manager::get)
    .def("find", (std::set<std::shared_ptr<smtk::resource::Resource>> (smtk::resource::Manager::*)(const std::string&)) &smtk::resource::Manager::find, py::arg("location"))
    .def("find", (std::set<std::shared_ptr<smtk::resource::Resource>> (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, bool)) &smtk::resource::Manager::find, py::arg("index"), py::arg("strict") = false)
    .def("metadata", [](smtk::resource::Manager& man) { std::vector<std::reference_wrapper<smtk::resource::Metadata>> vec; vec.reserve(man.metadata().size()); for (auto md : man.metadata()) { vec.emplace_back(md); } return vec; })
    .def("observers", (smtk::resource::Observers & (smtk::resource::Manager::*)()) &smtk::resource::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("observers", (smtk::resource::Observers const & (smtk::resource::Manager::*)() const) &smtk::resource::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("read", (smtk::resource::ResourcePtr (smtk::resource::Manager::*)(const smtk::resource::Resource::Index&, const std::string&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::resource::Manager::read)
    .def("read", (smtk::resource::ResourcePtr (smtk::resource::Manager::*)(const std::string&, const std::string&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::resource::Manager::read)
    .def("remove", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&)) &smtk::resource::Manager::remove)
    .def("resources", [](smtk::resource::Manager& man)
      {
        std::vector<std::shared_ptr<smtk::resource::Resource>> rsrcs;
        man.visit(
          [&rsrcs](smtk::resource::Resource& rsrc)
          {
            rsrcs.push_back(rsrc.shared_from_this());
            return smtk::common::Processing::CONTINUE;
          }
        );
        return rsrcs;
      }
    )
    .def("write", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::resource::Manager::write)
    .def("write", (bool (smtk::resource::Manager::*)(const std::shared_ptr<smtk::resource::Resource>&, const std::string&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::resource::Manager::write)
    ;
  return instance;
}

#endif
