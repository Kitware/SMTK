//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__smtk_extension_vtk_geometry_Geometry_h
#define pybind__smtk_extension_vtk_geometry_Geometry_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::extension::vtk::geometry::Geometry > pybind11_init_smtk_extension_vtk_geometry_Geometry(py::module &m)
{
  PySharedPtrClass< smtk::extension::vtk::geometry::Geometry > instance(m, "Geometry");
  instance
    .def("backend", &smtk::extension::vtk::geometry::Geometry::backend)
    .def("dimension", &smtk::extension::vtk::geometry::Geometry::dimension, py::arg("object"))
    // .def("purpose", &smtk::extension::vtk::geometry::Geometry::purpose, py::arg("object"))
    .def_static("addColorArray", &smtk::extension::vtk::geometry::Geometry::addColorArray, py::arg("data"), py::arg("color"), py::arg("name"))
    // .def("markModified", &smtk::extension::vtk::geometry::Geometry::markModified, py::arg("obj"))
    // .def("readLockRequired", &smtk::extension::vtk::geometry::Geometry::readLockRequired)
    // .def("resource", &smtk::extension::vtk::geometry::Geometry::resource)
    // .def("typeName", &smtk::extension::vtk::geometry::Geometry::typeName)
    // .def("visit", &smtk::extension::vtk::geometry::Geometry::visit, py::arg("fn"))
    // .def("Initial", [](){ return smtk::extension::vtk::geometry::Geometry::Initial; })
    // .def("Invalid", [](){ return smtk::extension::vtk::geometry::Geometry::Invalid; })
    ;
  return instance;
}

#endif
