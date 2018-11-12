//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_CreateDuct_h
#define pybind_smtk_session_rgg_operators_CreateDuct_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/CreateDuct.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::CreateDuct, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_CreateDuct(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::CreateDuct, smtk::operation::XMLOperation > instance(m, "CreateDuct");
  instance
    .def(py::init<::smtk::session::rgg::CreateDuct const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::CreateDuct & (smtk::session::rgg::CreateDuct::*)(::smtk::session::rgg::CreateDuct const &)) &smtk::session::rgg::CreateDuct::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateDuct> (*)()) &smtk::session::rgg::CreateDuct::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateDuct> (*)(::std::shared_ptr<smtk::session::rgg::CreateDuct> &)) &smtk::session::rgg::CreateDuct::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::CreateDuct> (smtk::session::rgg::CreateDuct::*)()) &smtk::session::rgg::CreateDuct::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::CreateDuct> (smtk::session::rgg::CreateDuct::*)() const) &smtk::session::rgg::CreateDuct::shared_from_this)
    ;
  return instance;
}

#endif
