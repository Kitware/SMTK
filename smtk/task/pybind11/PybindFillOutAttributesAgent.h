
//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_FillOutAttributesAgent_h
#define pybind_smtk_task_FillOutAttributesAgent_h

#include <pybind11/pybind11.h>

#include "smtk/task/FillOutAttributesAgent.h"

#include "smtk/task/State.h"

namespace py = pybind11;

inline py::class_< smtk::task::FillOutAttributesAgent, smtk::task::Agent > pybind11_init_smtk_task_FillOutAttributesAgent(py::module &m)
{
  py::class_< smtk::task::FillOutAttributesAgent, smtk::task::Agent > instance(m, "FillOutAttributesAgent");
  instance
    .def("typeName", &smtk::task::FillOutAttributesAgent::typeName)
    .def("inputPort", &smtk::task::FillOutAttributesAgent::inputPort)
    .def("outputPort", &smtk::task::FillOutAttributesAgent::outputPort)
    ;
  return instance;
}

#endif
