//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Instances_h
#define pybind_smtk_task_Instances_h

#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include "nlohmann/json.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Instances > pybind11_init_smtk_task_Instances(py::module &m)
{
  PySharedPtrClass< smtk::task::Instances > instance(m, "Instances");
  instance
    .def("createFromName",
      [](smtk::task::Instances& taskInstances, const std::string& name, const std::string& config)
      {
        std::shared_ptr<smtk::common::Managers> managers;
        smtk::task::Task::Configuration jConfig = nlohmann::json::parse(config);
        std::shared_ptr<smtk::task::Task> task = taskInstances.createFromName(name, jConfig, managers);
        return task;
      }
    )
    .def("createFromName",
      [](smtk::task::Instances& taskInstances, const std::string& name, const std::string& config, std::shared_ptr<smtk::common::Managers> managers)
      {
        smtk::task::Task::Configuration jConfig = nlohmann::json::parse(config);
        std::shared_ptr<smtk::task::Task> task = taskInstances.createFromName(name, jConfig, managers);
        return task;
      }
    )
    .def("clear", &smtk::task::Instances::clear)
    .def("instances",
      [](smtk::task::Instances& taskInstances)
      {
        std::vector<std::shared_ptr<smtk::task::Task>> taskList;
        taskList.reserve(taskInstances.size());
        taskInstances.visit(
          [&taskList](const std::shared_ptr<smtk::task::Task>& task)
          {
            if (task)
            {
              taskList.push_back(task);
            }
            return smtk::common::Visit::Continue;
          }
        );
        return taskList;
      }
    )
    ;
  return instance;
}

#endif
