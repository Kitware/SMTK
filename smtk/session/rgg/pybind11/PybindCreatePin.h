//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_CreatePin_h
#define pybind_smtk_session_rgg_operators_CreatePin_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/CreatePin.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::CreatePin, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_CreatePin(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::CreatePin, smtk::operation::XMLOperation > instance(m, "CreatePin");
  instance
    .def(py::init<::smtk::session::rgg::CreatePin const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::CreatePin & (smtk::session::rgg::CreatePin::*)(::smtk::session::rgg::CreatePin const &)) &smtk::session::rgg::CreatePin::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreatePin> (*)()) &smtk::session::rgg::CreatePin::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreatePin> (*)(::std::shared_ptr<smtk::session::rgg::CreatePin> &)) &smtk::session::rgg::CreatePin::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::CreatePin> (smtk::session::rgg::CreatePin::*)()) &smtk::session::rgg::CreatePin::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::CreatePin> (smtk::session::rgg::CreatePin::*)() const) &smtk::session::rgg::CreatePin::shared_from_this)
    ;
  return instance;
}

#endif
