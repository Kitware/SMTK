//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_JobAgent_h
#define pybind_smtk_job_JobAgent_h

#include "smtk/job/agents/JobAgent.h"

#include "smtk/task/Port.h"
#include "smtk/task/SubmitOperationAgent.h"
#include "smtk/task/Task.h"

#include "smtk/common/TypeContainer.h"

namespace py = pybind11;

inline py::class_<smtk::job::agents::JobAgent, smtk::task::SubmitOperationAgent>
pybind11_init_smtk_job_JobAgent(py::module &m)
{
  py::class_<smtk::job::agents::JobAgent, smtk::task::SubmitOperationAgent> instance(m, "JobAgent");
  instance
    .def_static("jobAgent", &smtk::job::agents::JobAgent::jobAgent, py::arg("task"), py::arg("agent_name"))
    .def_static("activeJobAgent", &smtk::job::agents::JobAgent::activeJobAgent, py::arg("app_context"), py::arg("agent_name"))

    .def("job", &smtk::job::agents::JobAgent::job)
    .def("inputPort", &smtk::job::agents::JobAgent::inputPort)
    ;
  return instance;
}

#endif
