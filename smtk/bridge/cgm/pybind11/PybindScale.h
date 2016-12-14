//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Scale_h
#define pybind_smtk_bridge_cgm_operators_Scale_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Scale.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Scale > pybind11_init_smtk_bridge_cgm_Scale(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Scale > instance(m, "Scale", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Scale const &>())
    .def("deepcopy", (smtk::bridge::cgm::Scale & (smtk::bridge::cgm::Scale::*)(::smtk::bridge::cgm::Scale const &)) &smtk::bridge::cgm::Scale::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Scale::baseCreate)
    .def("className", &smtk::bridge::cgm::Scale::className)
    .def("classname", &smtk::bridge::cgm::Scale::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Scale> (*)()) &smtk::bridge::cgm::Scale::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Scale> (*)(::std::shared_ptr<smtk::bridge::cgm::Scale> &)) &smtk::bridge::cgm::Scale::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Scale::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Scale> (smtk::bridge::cgm::Scale::*)() const) &smtk::bridge::cgm::Scale::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Scale> (smtk::bridge::cgm::Scale::*)()) &smtk::bridge::cgm::Scale::shared_from_this)
    ;
  return instance;
}

#endif
