//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_TriangulateFace_h
#define pybind_smtk_extension_delaunay_TriangulateFace_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/TriangulateFace.h"

PySharedPtrClass< smtk::model::TriangulateFace, smtk::model::Operator > pybind11_init_smtk_extension_delaunay_TriangulateFace(py::module &m)
{
  PySharedPtrClass< smtk::model::TriangulateFace, smtk::model::Operator > instance(m, "TriangulateFace");
  instance
    .def("classname", &smtk::model::TriangulateFace::classname)
    .def_static("create", (std::shared_ptr<smtk::model::TriangulateFace> (*)()) &smtk::model::TriangulateFace::create)
    .def_static("create", (std::shared_ptr<smtk::model::TriangulateFace> (*)(::std::shared_ptr<smtk::model::TriangulateFace> &)) &smtk::model::TriangulateFace::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::model::TriangulateFace> (smtk::model::TriangulateFace::*)()) &smtk::model::TriangulateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::TriangulateFace> (smtk::model::TriangulateFace::*)() const) &smtk::model::TriangulateFace::shared_from_this)
    .def("name", &smtk::model::TriangulateFace::name)
    .def("className", &smtk::model::TriangulateFace::className)
    .def_static("baseCreate", &smtk::model::TriangulateFace::baseCreate)
    .def("ableToOperate", &smtk::model::TriangulateFace::ableToOperate)
    ;
  return instance;
}

#endif
