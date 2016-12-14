//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateBody_h
#define pybind_smtk_bridge_cgm_operators_CreateBody_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateBody.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateBody > pybind11_init_smtk_bridge_cgm_CreateBody(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateBody > instance(m, "CreateBody", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateBody const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateBody & (smtk::bridge::cgm::CreateBody::*)(::smtk::bridge::cgm::CreateBody const &)) &smtk::bridge::cgm::CreateBody::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateBody::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateBody::className)
    .def("classname", &smtk::bridge::cgm::CreateBody::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateBody> (*)()) &smtk::bridge::cgm::CreateBody::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateBody> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateBody> &)) &smtk::bridge::cgm::CreateBody::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateBody::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateBody> (smtk::bridge::cgm::CreateBody::*)() const) &smtk::bridge::cgm::CreateBody::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateBody> (smtk::bridge::cgm::CreateBody::*)()) &smtk::bridge::cgm::CreateBody::shared_from_this)
    ;
  return instance;
}

#endif
