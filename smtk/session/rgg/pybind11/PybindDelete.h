//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_Delete_h
#define pybind_smtk_session_rgg_operators_Delete_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/Delete.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::Delete, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_Delete(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::Delete, smtk::operation::XMLOperation > instance(m, "Delete");
  instance
    .def(py::init<::smtk::session::rgg::Delete const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::Delete & (smtk::session::rgg::Delete::*)(::smtk::session::rgg::Delete const &)) &smtk::session::rgg::Delete::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Delete> (*)()) &smtk::session::rgg::Delete::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Delete> (*)(::std::shared_ptr<smtk::session::rgg::Delete> &)) &smtk::session::rgg::Delete::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::Delete> (smtk::session::rgg::Delete::*)()) &smtk::session::rgg::Delete::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::Delete> (smtk::session::rgg::Delete::*)() const) &smtk::session::rgg::Delete::shared_from_this)
    ;
  return instance;
}

#endif
