
//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_SubmitOperationAgent_h
#define pybind_smtk_task_SubmitOperationAgent_h

#include <pybind11/pybind11.h>

#include "smtk/task/SubmitOperationAgent.h"

#include "smtk/task/State.h"

namespace py = pybind11;

inline py::class_< smtk::task::SubmitOperationAgent, smtk::task::Agent > pybind11_init_smtk_task_SubmitOperationAgent(py::module &m)
{
  py::class_< smtk::task::SubmitOperationAgent, smtk::task::Agent > instance(m, "SubmitOperationAgent");
  instance
    .def("typeName", &smtk::task::SubmitOperationAgent::typeName)
    .def("operation", &smtk::task::SubmitOperationAgent::operation)
    ;
  return instance;
}

#endif
