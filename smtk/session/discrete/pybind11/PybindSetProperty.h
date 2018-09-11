//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_SetProperty_h
#define pybind_smtk_session_discrete_operators_SetProperty_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/SetProperty.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::SetProperty, smtk::operation::Operation > pybind11_init_smtk_session_discrete_SetProperty(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::SetProperty, smtk::operation::Operation > instance(m, "SetProperty");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::discrete::SetProperty const &>())
    .def("deepcopy", (smtk::session::discrete::SetProperty & (smtk::session::discrete::SetProperty::*)(::smtk::session::discrete::SetProperty const &)) &smtk::session::discrete::SetProperty::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::SetProperty> (*)()) &smtk::session::discrete::SetProperty::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::SetProperty> (*)(::std::shared_ptr<smtk::session::discrete::SetProperty> &)) &smtk::session::discrete::SetProperty::create, py::arg("ref"))
    .def("name", &smtk::session::discrete::SetProperty::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::SetProperty> (smtk::session::discrete::SetProperty::*)() const) &smtk::session::discrete::SetProperty::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::SetProperty> (smtk::session::discrete::SetProperty::*)()) &smtk::session::discrete::SetProperty::shared_from_this)
    ;
  return instance;
}

#endif
