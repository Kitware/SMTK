//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindBackend.h"
#include "PybindGeometry.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindVTKGeometry, geometry)
{
  geometry.doc() = "VTK geometry for resources and components.";

  py::module::import("smtk.geometry");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::extension::vtk::geometry::Backend > smtk_extension_vtk_geometry_Backend = pybind11_init_smtk_extension_vtk_geometry_Backend(geometry);
  py::class_< smtk::extension::vtk::geometry::Geometry > smtk_extension_vtk_geometry_Geometry = pybind11_init_smtk_extension_vtk_geometry_Geometry(geometry);
}
