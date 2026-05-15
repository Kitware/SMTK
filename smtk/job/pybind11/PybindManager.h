//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Manager_h
#define pybind_smtk_job_Manager_h

#include "smtk/job/DefinitionInstances.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Queue.h"
#include "smtk/common/pybind11/PybindInstances.h"
#include "smtk/common/pybind11/PybindActive.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::job::Manager> pybind11_init_smtk_job_Manager(py::module &m)
{
  // First, bind Instances and Active templates (specialized to the job manager).
  auto activeQueue = pybind11_init_smtk_common_Active<smtk::job::QueueInstances>(m, "ActiveQueue");
  // using QueueInstances = smtk::common::Instances<smtk::job::Queue>;
  // using ActiveQueue = smtk::common::Active<QueueInstances>;

  // Now bind the job manager:
  PySharedPtrClass<smtk::job::Manager> instance(m, "Manager");
  instance
    .def_static("create", (std::shared_ptr<smtk::job::Manager> (*)()) &smtk::job::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::job::Manager> (*)(::std::shared_ptr<smtk::job::Manager> &)) &smtk::job::Manager::create, py::arg("ref"))
    .def("jobTypes", [](smtk::job::Manager& jobManager) -> smtk::job::DefinitionInstances* { return &(jobManager.jobTypes()); }, py::return_value_policy::reference)
    .def("queues", [](smtk::job::Manager& jobManager) { return &(jobManager.queues()); }, py::return_value_policy::reference)
    .def("activeQueue", &smtk::job::Manager::activeQueue, py::return_value_policy::reference)
    .def("defaultQueue", &smtk::job::Manager::defaultQueue)
    .def("findQueueByName", [](smtk::job::Manager& manager, const std::string& name)
      {
        std::shared_ptr<smtk::job::Queue> queue = nullptr;
        manager.queues().visit([&queue, name](const std::shared_ptr<smtk::job::Queue>& qq) -> smtk::common::Visit
          {
            if (qq && qq->name() == name)
            {
              queue = qq;
              return smtk::common::Visit::Halt;
            }
            return smtk::common::Visit::Continue;
          }
        );
        return queue;
      }, py::arg("name"))
    ;
  return instance;
}

#endif
