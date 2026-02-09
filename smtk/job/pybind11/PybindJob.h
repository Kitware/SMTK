//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Job_h
#define pybind_smtk_job_Job_h

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::job::Job, smtk::resource::Component > pybind11_init_smtk_job_Job(py::module &m)
{
  PySharedPtrClass< smtk::job::Job, smtk::resource::Component > instance(m, "Job");
  instance
    .def_static("create", (std::shared_ptr<smtk::job::Job> (*)()) &smtk::job::Job::create)
    .def("queue", &smtk::job::Job::queue)
    .def("setQueue", &smtk::job::Job::setQueue, py::arg("queue"))

    .def("size", &smtk::job::Job::size)
    .def("setSize", &smtk::job::Job::setSize, py::arg("size"))

    .def("caseDirectory", &smtk::job::Job::caseDirectory)
    .def("setCaseDirectory", &smtk::job::Job::setCaseDirectory, py::arg("directory"))

    .def("caseDirectoryMountPoint", &smtk::job::Job::caseDirectoryMountPoint)
    .def("setCaseDirectoryMountPoint", &smtk::job::Job::setCaseDirectoryMountPoint, py::arg("mount_point"))

    .def("script", &smtk::job::Job::script)
    .def("setScript", &smtk::job::Job::setScript, py::arg("script"))

    .def("logs", &smtk::job::Job::logs)
    .def("setLogs", &smtk::job::Job::setLogs, py::arg("logs"))

    .def("queueId", &smtk::job::Job::queueId)
    .def("setQueueId", &smtk::job::Job::setQueueId, py::arg("queueId"))

    .def("schedule", &smtk::job::Job::schedule, py::arg("queue") = nullptr)
    .def("state", &smtk::job::Job::state)
    .def("status", &smtk::job::Job::status)

    .def("autoSchedule", &smtk::job::Job::autoSchedule)
    .def("setAutoSchedule", &smtk::job::Job::setAutoSchedule, py::arg("should_schedule"))

    .def("logParsers", &smtk::job::Job::logParsers)
    .def("setLogParser", &smtk::job::Job::setLogParser, py::arg("log_path"), py::arg("parser"))
    .def("clearLogParser", &smtk::job::Job::clearLogParser, py::arg("log_path"))
    .def("resetLogParsers", &smtk::job::Job::resetLogParsers)

    .def("linkTo", &smtk::job::Job::linkTo, py::arg("object"))
    .def("unlink", &smtk::job::Job::setQueue, py::arg("key"))
    .def("originators", &smtk::job::Job::originators)

    .def("containerImage", &smtk::job::Job::containerImage)
    .def("setContainerImage", &smtk::job::Job::setContainerImage, py::arg("url"))
    ;
  return instance;
}

#endif
