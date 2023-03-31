//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_State_h
#define pybind_smtk_task_State_h

#include <pybind11/pybind11.h>

#include "smtk/task/State.h"

namespace py = pybind11;

inline void pybind11_init_smtk_task_State(py::module &m)
{
  py::enum_<smtk::task::State>(m, "State")
    .value("Unavailable", smtk::task::State::Unavailable)
    .value("Incomplete", smtk::task::State::Incomplete)
    .value("Completable", smtk::task::State::Completable)
    .value("Completed", smtk::task::State::Completed)
    .export_values();
}

inline void pybind11_init_smtk_task_stateName(py::module &m)
{
  m.def("stateName", &smtk::task::stateName, "", py::arg("s"));
}

inline void pybind11_init_smtk_task_stateEnum(py::module &m)
{
  m.def("stateEnum", &smtk::task::stateEnum, "", py::arg("s"), py::arg("matched"));
}

#endif
