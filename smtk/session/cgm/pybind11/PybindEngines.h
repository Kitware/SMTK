//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_Engines_h
#define pybind_smtk_session_cgm_Engines_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/session/cgm/Engines.h"

namespace py = pybind11;

py::class_< smtk::session::cgm::Engines > pybind11_init_smtk_session_cgm_Engines(py::module &m)
{
  py::class_< smtk::session::cgm::Engines > instance(m, "Engines");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Engines const &>())
    .def("deepcopy", (smtk::session::cgm::Engines & (smtk::session::cgm::Engines::*)(::smtk::session::cgm::Engines const &)) &smtk::session::cgm::Engines::operator=)
    .def_static("areInitialized", &smtk::session::cgm::Engines::areInitialized)
    .def_static("isInitialized", &smtk::session::cgm::Engines::isInitialized, py::arg("engine"), py::arg("args") = std::vector<std::string>())
    .def_static("setDefault", &smtk::session::cgm::Engines::setDefault, py::arg("engine"))
    .def_static("currentEngine", &smtk::session::cgm::Engines::currentEngine)
    .def_static("listEngines", &smtk::session::cgm::Engines::listEngines)
    .def_static("shutdown", &smtk::session::cgm::Engines::shutdown)
    ;
  return instance;
}

#endif
