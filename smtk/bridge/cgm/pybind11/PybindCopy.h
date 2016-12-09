//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Copy_h
#define pybind_smtk_bridge_cgm_operators_Copy_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Copy.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Copy > pybind11_init_smtk_bridge_cgm_Copy(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Copy > instance(m, "Copy", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Copy const &>())
    .def("deepcopy", (smtk::bridge::cgm::Copy & (smtk::bridge::cgm::Copy::*)(::smtk::bridge::cgm::Copy const &)) &smtk::bridge::cgm::Copy::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Copy::baseCreate)
    .def("className", &smtk::bridge::cgm::Copy::className)
    .def("classname", &smtk::bridge::cgm::Copy::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Copy> (*)()) &smtk::bridge::cgm::Copy::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Copy> (*)(::std::shared_ptr<smtk::bridge::cgm::Copy> &)) &smtk::bridge::cgm::Copy::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Copy::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Copy> (smtk::bridge::cgm::Copy::*)() const) &smtk::bridge::cgm::Copy::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Copy> (smtk::bridge::cgm::Copy::*)()) &smtk::bridge::cgm::Copy::shared_from_this)
    ;
  return instance;
}

#endif
