//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_geometry_Backend_h
#define pybind_smtk_extension_vtk_geometry_Backend_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/geometry/Backend.h"

namespace py = pybind11;

inline py::class_< smtk::extension::vtk::geometry::Backend > pybind11_init_smtk_extension_vtk_geometry_Backend(py::module &m)
{
  py::class_< smtk::extension::vtk::geometry::Backend > instance(m, "Backend");
  instance
    .def("name", &smtk::extension::vtk::geometry::Backend::name)
    ;
  return instance;
}

#endif
