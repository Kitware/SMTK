//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__stage_source_smtk_4_smtk_geometry_Geometry_h
#define pybind__stage_source_smtk_4_smtk_geometry_Geometry_h

#include <pybind11/pybind11.h>

#include "smtk/geometry/Geometry.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/geometry/Backend.h"
#include "smtk/geometry/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::geometry::Geometry > pybind11_init_smtk_geometry_Geometry(py::module &m)
{
  PySharedPtrClass< smtk::geometry::Geometry > instance(m, "Geometry");
  instance
    .def("deepcopy", (smtk::geometry::Geometry & (smtk::geometry::Geometry::*)(::smtk::geometry::Geometry const &)) &smtk::geometry::Geometry::operator=)
    .def("backend", &smtk::geometry::Geometry::backend)
    .def("bounds", &smtk::geometry::Geometry::bounds, py::arg("arg0"), py::arg("bds"))
    .def("erase", &smtk::geometry::Geometry::erase, py::arg("uid"))
    .def("generationNumber", &smtk::geometry::Geometry::generationNumber, py::arg("arg0"))
    .def("markModified", &smtk::geometry::Geometry::markModified, py::arg("obj"))
    .def("readLockRequired", &smtk::geometry::Geometry::readLockRequired)
    .def("resource", &smtk::geometry::Geometry::resource)
    .def("typeName", &smtk::geometry::Geometry::typeName)
    .def("visit", &smtk::geometry::Geometry::visit, py::arg("fn"))
    .def_readonly_static("Initial", &smtk::geometry::Geometry::Initial)
    .def_readonly_static("Invalid", &smtk::geometry::Geometry::Invalid)
    .def_readonly_static("type_name", &smtk::geometry::Geometry::type_name)
    ;
  return instance;
}

#endif
