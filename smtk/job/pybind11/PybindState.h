//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_State_h
#define pybind_smtk_job_State_h

#include "smtk/job/State.h"

namespace py = pybind11;

inline void pybind11_init_smtk_job_State(py::module &m)
{
  py::enum_<smtk::job::State>(m, "State")
    .value("Unscheduled", smtk::job::State::Unscheduled)
    .value("Scheduled", smtk::job::State::Scheduled)
    .value("Running", smtk::job::State::Running)
    .value("Canceled", smtk::job::State::Canceled)
    .value("Completed", smtk::job::State::Completed)
    .export_values();
}

#endif // pybind_smtk_job_State_h
