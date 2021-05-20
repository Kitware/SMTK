//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__stage_source_smtk_4_smtk_geometry_Manager_h
#define pybind__stage_source_smtk_4_smtk_geometry_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/geometry/Manager.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::geometry::Manager > pybind11_init_smtk_geometry_Manager(py::module &m)
{
  PySharedPtrClass< smtk::geometry::Manager > instance(m, "Manager");
  instance
    .def_static("create", (std::shared_ptr<smtk::geometry::Manager> (*)()) &smtk::geometry::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::geometry::Manager> (*)(::std::shared_ptr<smtk::geometry::Manager> &)) &smtk::geometry::Manager::create, py::arg("ref"))
    .def("registerResourceManager", &smtk::geometry::Manager::registerResourceManager, py::arg("manager"))
    .def("visitBackends", &smtk::geometry::Manager::visitBackends, py::arg("visitor"))
    ;
  return instance;
}

#endif
