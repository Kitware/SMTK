//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_SetProperty_h
#define pybind_smtk_bridge_discrete_operators_SetProperty_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/SetProperty.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::SetProperty, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_SetProperty(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::SetProperty, smtk::model::Operator > instance(m, "SetProperty");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::discrete::SetProperty const &>())
    .def("deepcopy", (smtk::bridge::discrete::SetProperty & (smtk::bridge::discrete::SetProperty::*)(::smtk::bridge::discrete::SetProperty const &)) &smtk::bridge::discrete::SetProperty::operator=)
    .def_static("baseCreate", &smtk::bridge::discrete::SetProperty::baseCreate)
    .def("className", &smtk::bridge::discrete::SetProperty::className)
    .def("classname", &smtk::bridge::discrete::SetProperty::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::SetProperty> (*)()) &smtk::bridge::discrete::SetProperty::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::SetProperty> (*)(::std::shared_ptr<smtk::bridge::discrete::SetProperty> &)) &smtk::bridge::discrete::SetProperty::create, py::arg("ref"))
    .def("name", &smtk::bridge::discrete::SetProperty::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::SetProperty> (smtk::bridge::discrete::SetProperty::*)() const) &smtk::bridge::discrete::SetProperty::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::SetProperty> (smtk::bridge::discrete::SetProperty::*)()) &smtk::bridge::discrete::SetProperty::shared_from_this)
    ;
  return instance;
}

#endif
