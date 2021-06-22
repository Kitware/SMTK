//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_TaskNeedsResources_h
#define pybind_smtk_task_TaskNeedsResources_h

#include <pybind11/pybind11.h>

#include "smtk/task/TaskNeedsResources.h"

#include "smtk/task/Task.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::TaskNeedsResources, smtk::task::Task > pybind11_init_smtk_task_TaskNeedsResources(py::module &m)
{
  PySharedPtrClass< smtk::task::TaskNeedsResources, smtk::task::Task > instance(m, "TaskNeedsResources");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::task::Task::Configuration const &, ::smtk::common::Managers::Ptr const &>())
    .def(py::init<::smtk::task::Task::Configuration const &, ::smtk::task::Task::PassedDependencies const &, ::smtk::common::Managers::Ptr const &>())
    .def("configure", &smtk::task::TaskNeedsResources::configure, py::arg("config"))
    .def_static("create", (std::shared_ptr<smtk::task::TaskNeedsResources> (*)()) &smtk::task::TaskNeedsResources::create)
    .def_static("create", (std::shared_ptr<smtk::task::TaskNeedsResources> (*)(::std::shared_ptr<smtk::task::TaskNeedsResources> &)) &smtk::task::TaskNeedsResources::create, py::arg("ref"))
    .def("typeName", &smtk::task::TaskNeedsResources::typeName)
    .def_readonly_static("type_name", &smtk::task::TaskNeedsResources::type_name)
    ;
  py::class_< smtk::task::TaskNeedsResources::Predicate >(instance, "Predicate")
    .def(py::init<::smtk::task::TaskNeedsResources::Predicate const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::task::TaskNeedsResources::Predicate & (smtk::task::TaskNeedsResources::Predicate::*)(::smtk::task::TaskNeedsResources::Predicate const &)) &smtk::task::TaskNeedsResources::Predicate::operator=)
    .def_readwrite("m_role", &smtk::task::TaskNeedsResources::Predicate::m_role)
    .def_readwrite("m_minimumCount", &smtk::task::TaskNeedsResources::Predicate::m_minimumCount)
    .def_readwrite("m_maximumCount", &smtk::task::TaskNeedsResources::Predicate::m_maximumCount)
    .def_readwrite("m_type", &smtk::task::TaskNeedsResources::Predicate::m_type)
    .def_readwrite("m_validator", &smtk::task::TaskNeedsResources::Predicate::m_validator)
    .def_readwrite("m_resources", &smtk::task::TaskNeedsResources::Predicate::m_resources)
    ;
  return instance;
}

#endif
