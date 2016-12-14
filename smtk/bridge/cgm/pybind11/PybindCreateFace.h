//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateFace_h
#define pybind_smtk_bridge_cgm_operators_CreateFace_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateFace.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateFace > pybind11_init_smtk_bridge_cgm_CreateFace(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateFace > instance(m, "CreateFace", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateFace const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateFace & (smtk::bridge::cgm::CreateFace::*)(::smtk::bridge::cgm::CreateFace const &)) &smtk::bridge::cgm::CreateFace::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateFace::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateFace::className)
    .def("classname", &smtk::bridge::cgm::CreateFace::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateFace> (*)()) &smtk::bridge::cgm::CreateFace::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateFace> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateFace> &)) &smtk::bridge::cgm::CreateFace::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateFace::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateFace> (smtk::bridge::cgm::CreateFace::*)() const) &smtk::bridge::cgm::CreateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateFace> (smtk::bridge::cgm::CreateFace::*)()) &smtk::bridge::cgm::CreateFace::shared_from_this)
    ;
  return instance;
}

#endif
