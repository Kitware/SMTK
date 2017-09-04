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

PySharedPtrClass< smtk::mesh::TessellateFaces, smtk::model::Operator > pybind11_init_smtk_extension_delaunay_TessellateFaces(py::module &m)
{
  PySharedPtrClass< smtk::mesh::TessellateFaces, smtk::model::Operator > instance(m, "TessellateFaces");
  instance
    .def("classname", &smtk::mesh::TessellateFaces::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::TessellateFaces> (*)()) &smtk::mesh::TessellateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::TessellateFaces> (*)(::std::shared_ptr<smtk::mesh::TessellateFaces> &)) &smtk::mesh::TessellateFaces::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::TessellateFaces> (smtk::mesh::TessellateFaces::*)()) &smtk::mesh::TessellateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::TessellateFaces> (smtk::mesh::TessellateFaces::*)() const) &smtk::mesh::TessellateFaces::shared_from_this)
    .def("name", &smtk::mesh::TessellateFaces::name)
    .def("className", &smtk::mesh::TessellateFaces::className)
    .def_static("baseCreate", &smtk::mesh::TessellateFaces::baseCreate)
    .def("ableToOperate", &smtk::mesh::TessellateFaces::ableToOperate)
    ;
  return instance;
}

#endif
