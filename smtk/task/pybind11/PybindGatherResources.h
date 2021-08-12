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
    .def(py::init<>())
    .def(py::init<::smtk::task::Task::Configuration const &, ::smtk::common::Managers::Ptr const &>())
    .def(py::init<::smtk::task::Task::Configuration const &, ::smtk::task::Task::PassedDependencies const &, ::smtk::common::Managers::Ptr const &>())
    .def("configure", &smtk::task::GatherResources::configure, py::arg("config"))
    .def_static("create", (std::shared_ptr<smtk::task::GatherResources> (*)()) &smtk::task::GatherResources::create)
    .def_static("create", (std::shared_ptr<smtk::task::GatherResources> (*)(::std::shared_ptr<smtk::task::GatherResources> &)) &smtk::task::GatherResources::create, py::arg("ref"))
    .def("typeName", &smtk::task::GatherResources::typeName)
    .def_readonly_static("type_name", &smtk::task::GatherResources::type_name)
    ;
  py::class_< smtk::task::GatherResources::Predicate >(instance, "Predicate")
    .def(py::init<::smtk::task::GatherResources::Predicate const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::task::GatherResources::Predicate & (smtk::task::GatherResources::Predicate::*)(::smtk::task::GatherResources::Predicate const &)) &smtk::task::GatherResources::Predicate::operator=)
    .def_readwrite("m_role", &smtk::task::GatherResources::Predicate::m_role)
    .def_readwrite("m_minimumCount", &smtk::task::GatherResources::Predicate::m_minimumCount)
    .def_readwrite("m_maximumCount", &smtk::task::GatherResources::Predicate::m_maximumCount)
    .def_readwrite("m_type", &smtk::task::GatherResources::Predicate::m_type)
    .def_readwrite("m_validator", &smtk::task::GatherResources::Predicate::m_validator)
    .def_readwrite("m_resources", &smtk::task::GatherResources::Predicate::m_resources)
    ;
  return instance;
}

#endif
