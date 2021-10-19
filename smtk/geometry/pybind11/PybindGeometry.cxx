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

#include "PybindResource.h"
#include "PybindManager.h"
#include "PybindBackend.h"
#include "PybindGeometry.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindGeometry, geometry)
{
  geometry.doc() = "Geometric data for resources and components.";

  py::module::import("smtk.resource");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::geometry::Resource > smtk_geometry_Resource = pybind11_init_smtk_geometry_Resource(geometry);
  py::class_< smtk::geometry::Manager > smtk_geometry_Manager = pybind11_init_smtk_geometry_Manager(geometry);
  py::class_< smtk::geometry::Backend > smtk_geometry_Backend = pybind11_init_smtk_geometry_Backend(geometry);
  py::class_< smtk::geometry::Geometry > smtk_geometry_Geometry = pybind11_init_smtk_geometry_Geometry(geometry);
}
