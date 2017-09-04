//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_TriangulateFaces_h
#define pybind_smtk_extension_delaunay_TriangulateFaces_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

PySharedPtrClass< smtk::mesh::TriangulateFaces, smtk::model::Operator > pybind11_init_smtk_extension_delaunay_TriangulateFaces(py::module &m)
{
  PySharedPtrClass< smtk::mesh::TriangulateFaces, smtk::model::Operator > instance(m, "TriangulateFaces");
  instance
    .def("classname", &smtk::mesh::TriangulateFaces::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::TriangulateFaces> (*)()) &smtk::mesh::TriangulateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::TriangulateFaces> (*)(::std::shared_ptr<smtk::mesh::TriangulateFaces> &)) &smtk::mesh::TriangulateFaces::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::TriangulateFaces> (smtk::mesh::TriangulateFaces::*)()) &smtk::mesh::TriangulateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::TriangulateFaces> (smtk::mesh::TriangulateFaces::*)() const) &smtk::mesh::TriangulateFaces::shared_from_this)
    .def("name", &smtk::mesh::TriangulateFaces::name)
    .def("className", &smtk::mesh::TriangulateFaces::className)
    .def_static("baseCreate", &smtk::mesh::TriangulateFaces::baseCreate)
    .def("ableToOperate", &smtk::mesh::TriangulateFaces::ableToOperate)
    ;
  return instance;
}

#endif
