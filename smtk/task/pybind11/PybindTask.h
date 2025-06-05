//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Task_h
#define pybind_smtk_task_Task_h

#include <pybind11/pybind11.h>

#include "smtk/task/Task.h"

#include "smtk/task/State.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Task, smtk::resource::Component > pybind11_init_smtk_task_Task(py::module &m)
{
  PySharedPtrClass< smtk::task::Task, smtk::resource::Component > instance(m, "Task");
  instance
    .def("typeName", &smtk::task::Task::typeName)
    .def_static("create", (std::shared_ptr<smtk::task::Task> (*)()) &smtk::task::Task::create)
    .def("configure", [](smtk::task::Task& task, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        task.configure(config);
      })
    .def("name", &smtk::task::Task::name)
    .def("setName", &smtk::task::Task::setName, py::arg("name"))
    .def("title", &smtk::task::Task::name)
    .def("setTitle", &smtk::task::Task::setName, py::arg("title"))
    .def("ports", &smtk::task::Task::ports)
    .def("portData", &smtk::task::Task::portData, py::arg("port"))
    .def("state", &smtk::task::Task::state)
    .def("agents", &smtk::task::Task::agents, py::return_value_policy::reference_internal)
    .def("children", &smtk::task::Task::children, py::return_value_policy::reference_internal)
    .def("markCompleted", &smtk::task::Task::markCompleted, py::arg("completed"))
    .def("dependencies", &smtk::task::Task::dependencies)
    .def("addDependency", &smtk::task::Task::addDependency, py::arg("dependency"))
    .def("removeDependency", &smtk::task::Task::removeDependency, py::arg("dependency"))
    .def("observers", &smtk::task::Task::observers)
    //.def("internalState", &smtk::task::Task::internalState)
    ;
  return instance;
}

#endif
