//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreatePrism_h
#define pybind_smtk_bridge_cgm_operators_CreatePrism_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreatePrism.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreatePrism > pybind11_init_smtk_bridge_cgm_CreatePrism(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreatePrism > instance(m, "CreatePrism", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreatePrism const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreatePrism & (smtk::bridge::cgm::CreatePrism::*)(::smtk::bridge::cgm::CreatePrism const &)) &smtk::bridge::cgm::CreatePrism::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreatePrism::baseCreate)
    .def("className", &smtk::bridge::cgm::CreatePrism::className)
    .def("classname", &smtk::bridge::cgm::CreatePrism::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreatePrism> (*)()) &smtk::bridge::cgm::CreatePrism::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreatePrism> (*)(::std::shared_ptr<smtk::bridge::cgm::CreatePrism> &)) &smtk::bridge::cgm::CreatePrism::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreatePrism::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreatePrism> (smtk::bridge::cgm::CreatePrism::*)() const) &smtk::bridge::cgm::CreatePrism::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreatePrism> (smtk::bridge::cgm::CreatePrism::*)()) &smtk::bridge::cgm::CreatePrism::shared_from_this)
    ;
  return instance;
}

#endif
