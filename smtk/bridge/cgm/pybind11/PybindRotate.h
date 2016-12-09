//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Rotate_h
#define pybind_smtk_bridge_cgm_operators_Rotate_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Rotate.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Rotate > pybind11_init_smtk_bridge_cgm_Rotate(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Rotate > instance(m, "Rotate", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Rotate const &>())
    .def("deepcopy", (smtk::bridge::cgm::Rotate & (smtk::bridge::cgm::Rotate::*)(::smtk::bridge::cgm::Rotate const &)) &smtk::bridge::cgm::Rotate::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Rotate::baseCreate)
    .def("className", &smtk::bridge::cgm::Rotate::className)
    .def("classname", &smtk::bridge::cgm::Rotate::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Rotate> (*)()) &smtk::bridge::cgm::Rotate::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Rotate> (*)(::std::shared_ptr<smtk::bridge::cgm::Rotate> &)) &smtk::bridge::cgm::Rotate::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Rotate::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Rotate> (smtk::bridge::cgm::Rotate::*)() const) &smtk::bridge::cgm::Rotate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Rotate> (smtk::bridge::cgm::Rotate::*)()) &smtk::bridge::cgm::Rotate::shared_from_this)
    ;
  return instance;
}

#endif
