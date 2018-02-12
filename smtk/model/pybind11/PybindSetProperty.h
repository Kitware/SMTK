//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_SetProperty_h
#define pybind_smtk_model_operators_SetProperty_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/SetProperty.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::SetProperty, smtk::operation::XMLOperation > pybind11_init_smtk_model_SetProperty(py::module &m)
{
  PySharedPtrClass< smtk::model::SetProperty, smtk::operation::XMLOperation > instance(m, "SetProperty");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::SetProperty const &>())
    .def("deepcopy", (smtk::model::SetProperty & (smtk::model::SetProperty::*)(::smtk::model::SetProperty const &)) &smtk::model::SetProperty::operator=)
    .def("classname", &smtk::model::SetProperty::classname)
    .def_static("create", (std::shared_ptr<smtk::model::SetProperty> (*)()) &smtk::model::SetProperty::create)
    .def_static("create", (std::shared_ptr<smtk::model::SetProperty> (*)(::std::shared_ptr<smtk::model::SetProperty> &)) &smtk::model::SetProperty::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::SetProperty> (smtk::model::SetProperty::*)() const) &smtk::model::SetProperty::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::SetProperty> (smtk::model::SetProperty::*)()) &smtk::model::SetProperty::shared_from_this)
    ;
  return instance;
}

#endif
