//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Status_h
#define pybind_smtk_job_Status_h

#include "smtk/job/Status.h"

namespace py = pybind11;

inline void pybind11_init_smtk_job_Status(py::module &m)
{
  py::enum_<smtk::job::Status>(m, "Status")
    .value("Pending", smtk::job::Status::Pending)
    .value("Succeeded", smtk::job::Status::Succeeded)
    .value("Failed", smtk::job::Status::Failed)
    .value("Terminated", smtk::job::Status::Terminated)
    .export_values();
}

#endif // pybind_smtk_job_Status_h
