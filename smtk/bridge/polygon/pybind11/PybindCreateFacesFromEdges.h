//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateFacesFromEdges_h
#define pybind_smtk_bridge_polygon_operators_CreateFacesFromEdges_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateFacesFromEdges > pybind11_init_smtk_bridge_polygon_CreateFacesFromEdges(py::module &m, PySharedPtrClass< smtk::bridge::polygon::CreateFaces >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateFacesFromEdges > instance(m, "CreateFacesFromEdges", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateFacesFromEdges const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateFacesFromEdges & (smtk::bridge::polygon::CreateFacesFromEdges::*)(::smtk::bridge::polygon::CreateFacesFromEdges const &)) &smtk::bridge::polygon::CreateFacesFromEdges::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateFacesFromEdges::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateFacesFromEdges::className)
    .def("classname", &smtk::bridge::polygon::CreateFacesFromEdges::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateFacesFromEdges> (*)()) &smtk::bridge::polygon::CreateFacesFromEdges::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateFacesFromEdges> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateFacesFromEdges> &)) &smtk::bridge::polygon::CreateFacesFromEdges::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateFacesFromEdges::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateFacesFromEdges> (smtk::bridge::polygon::CreateFacesFromEdges::*)() const) &smtk::bridge::polygon::CreateFacesFromEdges::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateFacesFromEdges> (smtk::bridge::polygon::CreateFacesFromEdges::*)()) &smtk::bridge::polygon::CreateFacesFromEdges::shared_from_this)
    ;
  return instance;
}

#endif
