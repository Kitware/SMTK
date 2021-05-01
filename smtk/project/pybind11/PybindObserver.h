//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Observer_h
#define pybind_smtk_project_Observer_h

#include <pybind11/pybind11.h>

#include "smtk/project/Observer.h"

namespace py = pybind11;

void pybind11_init_smtk_project_EventType(py::module &m)
{
  py::enum_<smtk::project::EventType>(m, "EventType")
    .value("ADDED", smtk::project::EventType::ADDED)
    .value("MODIFIED", smtk::project::EventType::MODIFIED)
    .value("REMOVED", smtk::project::EventType::REMOVED)
    .export_values();
}

#endif
