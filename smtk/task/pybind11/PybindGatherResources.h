//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_GatherResources_h
#define pybind_smtk_task_GatherResources_h

#include <pybind11/pybind11.h>

#include "smtk/task/GatherResources.h"

#include "smtk/task/Task.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::GatherResources, smtk::task::Task > pybind11_init_smtk_task_GatherResources(py::module &m)
{
  PySharedPtrClass< smtk::task::GatherResources, smtk::task::Task > instance(m, "GatherResources");
  instance
    .def("configure", [](smtk::task::Task& task, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        task.configure(config);
      }, py::arg("config"))
    .def_static("create", (std::shared_ptr<smtk::task::GatherResources> (*)()) &smtk::task::GatherResources::create)
    .def_static("create", (std::shared_ptr<smtk::task::GatherResources> (*)(::std::shared_ptr<smtk::task::GatherResources> &)) &smtk::task::GatherResources::create, py::arg("ref"))
    .def("typeName", &smtk::task::GatherResources::typeName)
    .def_readonly_static("type_name", &smtk::task::GatherResources::type_name)
    ;
  py::class_< smtk::task::GatherResources::ResourceSet >(instance, "ResourceSet")
    .def(py::init<::smtk::task::GatherResources::ResourceSet const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::task::GatherResources::ResourceSet & (smtk::task::GatherResources::ResourceSet::*)(::smtk::task::GatherResources::ResourceSet const &)) &smtk::task::GatherResources::ResourceSet::operator=)
    .def_readwrite("m_role", &smtk::task::GatherResources::ResourceSet::m_role)
    .def_readwrite("m_minimumCount", &smtk::task::GatherResources::ResourceSet::m_minimumCount)
    .def_readwrite("m_maximumCount", &smtk::task::GatherResources::ResourceSet::m_maximumCount)
    .def_readwrite("m_type", &smtk::task::GatherResources::ResourceSet::m_type)
    .def_readwrite("m_validator", &smtk::task::GatherResources::ResourceSet::m_validator)
    .def_readwrite("m_resources", &smtk::task::GatherResources::ResourceSet::m_resources)
    ;
  return instance;
}

#endif
