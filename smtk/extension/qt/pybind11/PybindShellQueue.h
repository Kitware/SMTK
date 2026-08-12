//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_ShellQueue_h
#define pybind_smtk_job_ShellQueue_h

#include "smtk/extension/qt/job/ShellQueue.h"

#include "smtk/job/Manager.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::qt::job::ShellQueue, smtk::job::DatabaseQueue, smtk::job::Queue>
pybind11_init_smtk_qt_job_ShellQueue(py::module &m)
{
  PySharedPtrClass<smtk::qt::job::ShellQueue, smtk::job::DatabaseQueue, smtk::job::Queue> instance(m, "ShellQueue");
  instance
    .def_static("create_or_restore", [](
        const std::string& name,
        const std::string& description,
        const std::string& location,
        int maxJobSize,
        const std::unordered_set<std::string>& tags,
        bool removeQueueOnDestruction,
        const std::string& uid,
        const std::shared_ptr<smtk::resource::Manager>& resourceManager,
        const std::shared_ptr<smtk::operation::Manager>& operationManager,
        const std::shared_ptr<smtk::job::Manager>& jobManager
      )
      {
        std::unordered_set<smtk::string::Token> tokenTags;
        for (const auto& tag : tags) { tokenTags.insert(tag); }
        return smtk::job::DatabaseQueue::createOrRestore<smtk::qt::job::ShellQueue>(
          name, description, location, maxJobSize, tokenTags, removeQueueOnDestruction,
          smtk::common::UUID(uid), resourceManager, operationManager, jobManager);
      },
      py::arg("name"), py::arg("description"), py::arg("location"),
      py::arg("max_job_size") = 0,
      py::arg("tags") = std::unordered_set<std::string>(),
      py::arg("remove_queue_on_destruction") = false,
      py::arg("uid") = std::string(),
      py::arg("resource_manager") = std::shared_ptr<smtk::resource::Manager>(),
      py::arg("operation_manager") = std::shared_ptr<smtk::operation::Manager>(),
      py::arg("job_manager") = std::shared_ptr<smtk::job::Manager>()
    )
    .def("setInterpreter", [](smtk::qt::job::ShellQueue& self, const std::string& path)
      {
        self.setInterpreter(path);
      }, py::arg("path"))
    .def("interpreter", [](const smtk::qt::job::ShellQueue& self)
      {
        return self.interpreter().string();
      })
    .def("setInterpreterArguments", &smtk::qt::job::ShellQueue::setInterpreterArguments,
      py::arg("arguments"))
    .def("interpreterArguments", &smtk::qt::job::ShellQueue::interpreterArguments)
    ;
  return instance;
}

#endif
