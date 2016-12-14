//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateSphere_h
#define pybind_smtk_bridge_cgm_operators_CreateSphere_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateSphere.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateSphere > pybind11_init_smtk_bridge_cgm_CreateSphere(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateSphere > instance(m, "CreateSphere", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateSphere const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateSphere & (smtk::bridge::cgm::CreateSphere::*)(::smtk::bridge::cgm::CreateSphere const &)) &smtk::bridge::cgm::CreateSphere::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateSphere::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateSphere::className)
    .def("classname", &smtk::bridge::cgm::CreateSphere::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateSphere> (*)()) &smtk::bridge::cgm::CreateSphere::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateSphere> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateSphere> &)) &smtk::bridge::cgm::CreateSphere::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateSphere::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateSphere> (smtk::bridge::cgm::CreateSphere::*)() const) &smtk::bridge::cgm::CreateSphere::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateSphere> (smtk::bridge::cgm::CreateSphere::*)()) &smtk::bridge::cgm::CreateSphere::shared_from_this)
    ;
  return instance;
}

#endif
