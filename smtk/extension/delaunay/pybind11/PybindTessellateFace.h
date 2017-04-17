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

PySharedPtrClass< smtk::model::TessellateFace, smtk::model::Operator > pybind11_init_smtk_extension_delaunay_TessellateFace(py::module &m)
{
  PySharedPtrClass< smtk::model::TessellateFace, smtk::model::Operator > instance(m, "TessellateFace");
  instance
    .def("classname", &smtk::model::TessellateFace::classname)
    .def_static("create", (std::shared_ptr<smtk::model::TessellateFace> (*)()) &smtk::model::TessellateFace::create)
    .def_static("create", (std::shared_ptr<smtk::model::TessellateFace> (*)(::std::shared_ptr<smtk::model::TessellateFace> &)) &smtk::model::TessellateFace::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::model::TessellateFace> (smtk::model::TessellateFace::*)()) &smtk::model::TessellateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::TessellateFace> (smtk::model::TessellateFace::*)() const) &smtk::model::TessellateFace::shared_from_this)
    .def("name", &smtk::model::TessellateFace::name)
    .def("className", &smtk::model::TessellateFace::className)
    .def_static("baseCreate", &smtk::model::TessellateFace::baseCreate)
    .def("ableToOperate", &smtk::model::TessellateFace::ableToOperate)
    ;
  return instance;
}

#endif
