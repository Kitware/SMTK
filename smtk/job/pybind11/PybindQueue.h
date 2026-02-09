//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Queue_h
#define pybind_smtk_job_Queue_h

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::job::Queue> pybind11_init_smtk_job_Queue(py::module &m)
{
  PySharedPtrClass<smtk::job::Queue> instance(m, "Queue");
  instance
    .def("name", &smtk::job::Queue::name)
    .def("description", &smtk::job::Queue::description)
    .def("location", &smtk::job::Queue::location)
    .def("maximumJobSize", &smtk::job::Queue::maximumJobSize)
    .def("tags", &smtk::job::Queue::tags)

    .def("schedule", &smtk::job::Queue::schedule, py::arg("job"))
    .def("cancel", &smtk::job::Queue::cancel, py::arg("job"))
    .def("jobState", &smtk::job::Queue::jobState, py::arg("job"))
    .def("jobStatus", &smtk::job::Queue::jobStatus, py::arg("job"))
    .def("allJobs", &smtk::job::Queue::allJobs)
    ;
  return instance;
}

#endif
