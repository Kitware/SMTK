//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_DefinitionInstances_h
#define pybind_smtk_job_DefinitionInstances_h

#include "smtk/job/DefinitionInstances.h"
#include "smtk/common/pybind11/PybindInstances.h"

namespace py = pybind11;

inline py::class_<smtk::job::DefinitionInstances, smtk::common::Instances<smtk::job::Definition>>
pybind11_init_smtk_job_DefinitionInstances(py::module &m)
{
  // Register the templated base class:
  auto defInstBase = pybind11_init_smtk_common_Instances<smtk::job::Definition>(m, "DefinitionInstancesBase");
  py::class_<smtk::job::DefinitionInstances, smtk::common::Instances<smtk::job::Definition>> instance(m, "DefinitionInstances");
  instance
    .def("findByName", &smtk::job::DefinitionInstances::findByName, py::arg("name"))
    ;
  return instance;
}

#endif
