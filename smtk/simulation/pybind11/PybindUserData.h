//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_simulation_UserData_h
#define pybind_smtk_simulation_UserData_h

#include <pybind11/pybind11.h>

#include "smtk/simulation/UserData.h"

namespace py = pybind11;

py::class_< smtk::simulation::UserData > pybind11_init_smtk_simulation_UserData(py::module &m)
{
  py::class_< smtk::simulation::UserData > instance(m, "UserData");
  instance
    .def(py::init<::smtk::simulation::UserData const &>())
    .def("deepcopy", (smtk::simulation::UserData & (smtk::simulation::UserData::*)(::smtk::simulation::UserData const &)) &smtk::simulation::UserData::operator=)
    .def_static("New", &smtk::simulation::UserData::New)
    ;
  return instance;
}

#endif
