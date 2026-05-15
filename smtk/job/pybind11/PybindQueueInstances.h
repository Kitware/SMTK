//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_QueueInstances_h
#define pybind_smtk_job_QueueInstances_h

#include "smtk/job/QueueInstances.h"
#include "smtk/common/pybind11/PybindInstances.h"

namespace py = pybind11;

inline py::class_<smtk::job::QueueInstances, smtk::common::Instances<smtk::job::Queue>>
pybind11_init_smtk_job_QueueInstances(py::module &m)
{
  // Register the templated base class:
  auto defInstBase = pybind11_init_smtk_common_Instances<smtk::job::Queue>(m, "QueueInstancesBase");
  py::class_<smtk::job::QueueInstances, smtk::common::Instances<smtk::job::Queue>> instance(m, "QueueInstances");
  instance
    .def("findByName", &smtk::job::QueueInstances::findByName, py::arg("name"))
    ;
  return instance;
}

#endif
