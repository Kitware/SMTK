//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_Engines_h
#define pybind_smtk_bridge_cgm_Engines_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/bridge/cgm/Engines.h"

namespace py = pybind11;

py::class_< smtk::bridge::cgm::Engines > pybind11_init_smtk_bridge_cgm_Engines(py::module &m)
{
  py::class_< smtk::bridge::cgm::Engines > instance(m, "Engines");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Engines const &>())
    .def("deepcopy", (smtk::bridge::cgm::Engines & (smtk::bridge::cgm::Engines::*)(::smtk::bridge::cgm::Engines const &)) &smtk::bridge::cgm::Engines::operator=)
    .def_static("areInitialized", &smtk::bridge::cgm::Engines::areInitialized)
    .def_static("isInitialized", &smtk::bridge::cgm::Engines::isInitialized, py::arg("engine"), py::arg("args") = std::vector<std::string>())
    .def_static("setDefault", &smtk::bridge::cgm::Engines::setDefault, py::arg("engine"))
    .def_static("currentEngine", &smtk::bridge::cgm::Engines::currentEngine)
    .def_static("listEngines", &smtk::bridge::cgm::Engines::listEngines)
    .def_static("shutdown", &smtk::bridge::cgm::Engines::shutdown)
    ;
  return instance;
}

#endif
