//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_TessellateFace_h
#define pybind_smtk_extension_delaunay_TessellateFace_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/TessellateFace.h"

PySharedPtrClass< smtk::mesh::TessellateFace, smtk::model::Operator > pybind11_init_smtk_extension_delaunay_TessellateFace(py::module &m)
{
  PySharedPtrClass< smtk::mesh::TessellateFace, smtk::model::Operator > instance(m, "TessellateFace");
  instance
    .def("classname", &smtk::mesh::TessellateFace::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::TessellateFace> (*)()) &smtk::mesh::TessellateFace::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::TessellateFace> (*)(::std::shared_ptr<smtk::mesh::TessellateFace> &)) &smtk::mesh::TessellateFace::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::TessellateFace> (smtk::mesh::TessellateFace::*)()) &smtk::mesh::TessellateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::TessellateFace> (smtk::mesh::TessellateFace::*)() const) &smtk::mesh::TessellateFace::shared_from_this)
    .def("name", &smtk::mesh::TessellateFace::name)
    .def("className", &smtk::mesh::TessellateFace::className)
    .def_static("baseCreate", &smtk::mesh::TessellateFace::baseCreate)
    .def("ableToOperate", &smtk::mesh::TessellateFace::ableToOperate)
    ;
  return instance;
}

#endif
