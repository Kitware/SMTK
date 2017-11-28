//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Event_h
#define pybind_smtk_resource_Event_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Event.h"

namespace py = pybind11;

void pybind11_init_smtk_resource_Event(py::module &m)
{
  py::enum_<smtk::resource::Event>(m, "Event")
    .value("RESOURCE_ADDED", smtk::resource::Event::RESOURCE_ADDED)
    .value("RESOURCE_REMOVED", smtk::resource::Event::RESOURCE_REMOVED)
    .export_values();
}

#endif
