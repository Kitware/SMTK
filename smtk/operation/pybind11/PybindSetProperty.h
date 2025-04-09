//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_operators_SetProperty_h
#define pybind_smtk_operation_operators_SetProperty_h

#include <pybind11/pybind11.h>

#include "smtk/operation/operators/SetProperty.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::SetProperty, smtk::operation::XMLOperation > pybind11_init_smtk_operation_SetProperty(py::module &m)
{
  PySharedPtrClass< smtk::operation::SetProperty, smtk::operation::XMLOperation > instance(m, "SetProperty");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::operation::SetProperty> (*)()) &smtk::operation::SetProperty::create)
    .def_static("create", (std::shared_ptr<smtk::operation::SetProperty> (*)(::std::shared_ptr<smtk::operation::SetProperty> &)) &smtk::operation::SetProperty::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::SetProperty> (smtk::operation::SetProperty::*)() const) &smtk::operation::SetProperty::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::SetProperty> (smtk::operation::SetProperty::*)()) &smtk::operation::SetProperty::shared_from_this)
    ;
  return instance;
}

#endif
