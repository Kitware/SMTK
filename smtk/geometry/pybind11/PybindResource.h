//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__stage_source_smtk_4_smtk_geometry_Resource_h
#define pybind__stage_source_smtk_4_smtk_geometry_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/geometry/Resource.h"

#include "smtk/geometry/Backend.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;


inline PySharedPtrClass< smtk::geometry::Resource, smtk::resource::Resource > pybind11_init_smtk_geometry_Resource(py::module &m)
{
  PySharedPtrClass< smtk::geometry::Resource, smtk::resource::Resource > instance(m, "Resource");
  instance
    // .def("geometry", &smtk::geometry::Resource::geometry, py::arg("backend"))
    .def("shared_from_this", (std::shared_ptr<const smtk::geometry::Resource> (smtk::geometry::Resource::*)() const) &smtk::geometry::Resource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::geometry::Resource> (smtk::geometry::Resource::*)()) &smtk::geometry::Resource::shared_from_this)
    .def("typeName", &smtk::geometry::Resource::typeName)
    // .def("visitGeometry", &smtk::geometry::Resource::visitGeometry, py::arg("visitor"))
    ;
  return instance;
}

#endif
