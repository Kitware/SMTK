//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_geometry_Backend_h
#define pybind_smtk_geometry_Backend_h

#include <pybind11/pybind11.h>

#include "smtk/geometry/Backend.h"

namespace py = pybind11;

inline py::class_< smtk::geometry::Backend > pybind11_init_smtk_geometry_Backend(py::module &m)
{
  py::class_< smtk::geometry::Backend > instance(m, "Backend");
  instance
    .def("deepcopy", (smtk::geometry::Backend & (smtk::geometry::Backend::*)(::smtk::geometry::Backend const &)) &smtk::geometry::Backend::operator=)
    .def("index", &smtk::geometry::Backend::index)
    .def("name", &smtk::geometry::Backend::name)
    ;
  return instance;
}

#endif
