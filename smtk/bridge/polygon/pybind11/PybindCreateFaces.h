//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateFaces_h
#define pybind_smtk_bridge_polygon_operators_CreateFaces_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateFaces.h"

namespace py = pybind11;

py::class_< smtk::bridge::polygon::ModelEdgeInfo > pybind11_init_smtk_bridge_polygon_ModelEdgeInfo(py::module &m)
{
  py::class_< smtk::bridge::polygon::ModelEdgeInfo > instance(m, "ModelEdgeInfo");
  instance
    .def(py::init<>())
    .def(py::init<int>())
    .def(py::init<::smtk::bridge::polygon::ModelEdgeInfo const &>())
    .def("deepcopy", (smtk::bridge::polygon::ModelEdgeInfo & (smtk::bridge::polygon::ModelEdgeInfo::*)(::smtk::bridge::polygon::ModelEdgeInfo const &)) &smtk::bridge::polygon::ModelEdgeInfo::operator=)
    .def_readwrite("m_allowedOrientations", &smtk::bridge::polygon::ModelEdgeInfo::m_allowedOrientations)
    ;
  return instance;
}

PySharedPtrClass< smtk::bridge::polygon::CreateFaces > pybind11_init_smtk_bridge_polygon_CreateFaces(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateFaces > instance(m, "CreateFaces", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::CreateFaces const &>())
    .def("deepcopy", (smtk::bridge::polygon::CreateFaces & (smtk::bridge::polygon::CreateFaces::*)(::smtk::bridge::polygon::CreateFaces const &)) &smtk::bridge::polygon::CreateFaces::operator=)
    .def_static("baseCreate", &smtk::bridge::polygon::CreateFaces::baseCreate)
    .def("className", &smtk::bridge::polygon::CreateFaces::className)
    .def("classname", &smtk::bridge::polygon::CreateFaces::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateFaces> (*)()) &smtk::bridge::polygon::CreateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateFaces> (*)(::std::shared_ptr<smtk::bridge::polygon::CreateFaces> &)) &smtk::bridge::polygon::CreateFaces::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::CreateFaces::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::CreateFaces> (smtk::bridge::polygon::CreateFaces::*)() const) &smtk::bridge::polygon::CreateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::CreateFaces> (smtk::bridge::polygon::CreateFaces::*)()) &smtk::bridge::polygon::CreateFaces::shared_from_this)
    ;
  return instance;
}

#endif
