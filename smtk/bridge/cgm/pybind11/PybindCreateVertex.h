//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateVertex_h
#define pybind_smtk_bridge_cgm_operators_CreateVertex_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateVertex.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateVertex > pybind11_init_smtk_bridge_cgm_CreateVertex(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateVertex > instance(m, "CreateVertex", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateVertex const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateVertex & (smtk::bridge::cgm::CreateVertex::*)(::smtk::bridge::cgm::CreateVertex const &)) &smtk::bridge::cgm::CreateVertex::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateVertex::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateVertex::className)
    .def("classname", &smtk::bridge::cgm::CreateVertex::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateVertex> (*)()) &smtk::bridge::cgm::CreateVertex::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateVertex> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateVertex> &)) &smtk::bridge::cgm::CreateVertex::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateVertex::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateVertex> (smtk::bridge::cgm::CreateVertex::*)() const) &smtk::bridge::cgm::CreateVertex::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateVertex> (smtk::bridge::cgm::CreateVertex::*)()) &smtk::bridge::cgm::CreateVertex::shared_from_this)
    ;
  return instance;
}

#endif
