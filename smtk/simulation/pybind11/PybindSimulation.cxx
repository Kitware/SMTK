//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindExportSpec.h"
#include "PybindUserData.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindSimulation)
{
  py::module simulation("_smtkPybindSimulation", "<description>");
  // py::module smtk = m.def_submodule("smtk", "<description>");
  // py::module simulation = smtk.def_submodule("simulation", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::simulation::ExportSpec > smtk_simulation_ExportSpec = pybind11_init_smtk_simulation_ExportSpec(simulation);
  py::class_< smtk::simulation::UserData > smtk_simulation_UserData = pybind11_init_smtk_simulation_UserData(simulation);

  return simulation.ptr();
}
