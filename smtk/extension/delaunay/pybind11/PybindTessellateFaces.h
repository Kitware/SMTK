//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_TessellateFaces_h
#define pybind_smtk_extension_delaunay_TessellateFaces_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/TessellateFaces.h"

inline PySharedPtrClass< smtk::extension::delaunay::TessellateFaces, smtk::operation::XMLOperation > pybind11_init_smtk_extension_delaunay_TessellateFaces(py::module &m)
{
  PySharedPtrClass< smtk::extension::delaunay::TessellateFaces, smtk::operation::XMLOperation > instance(m, "TessellateFaces");
  instance
    .def_static("create", (std::shared_ptr<smtk::extension::delaunay::TessellateFaces> (*)()) &smtk::extension::delaunay::TessellateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::extension::delaunay::TessellateFaces> (*)(::std::shared_ptr<smtk::extension::delaunay::TessellateFaces> &)) &smtk::extension::delaunay::TessellateFaces::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::extension::delaunay::TessellateFaces> (smtk::extension::delaunay::TessellateFaces::*)()) &smtk::extension::delaunay::TessellateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::extension::delaunay::TessellateFaces> (smtk::extension::delaunay::TessellateFaces::*)() const) &smtk::extension::delaunay::TessellateFaces::shared_from_this)
    .def("ableToOperate", &smtk::extension::delaunay::TessellateFaces::ableToOperate)
    ;
  return instance;
}

#endif
