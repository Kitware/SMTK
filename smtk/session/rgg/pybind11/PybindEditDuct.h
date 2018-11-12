//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_EditDuct_h
#define pybind_smtk_session_rgg_operators_EditDuct_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/EditDuct.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::EditDuct, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_EditDuct(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::EditDuct, smtk::operation::XMLOperation > instance(m, "EditDuct");
  instance
    .def(py::init<::smtk::session::rgg::EditDuct const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::EditDuct & (smtk::session::rgg::EditDuct::*)(::smtk::session::rgg::EditDuct const &)) &smtk::session::rgg::EditDuct::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditDuct> (*)()) &smtk::session::rgg::EditDuct::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditDuct> (*)(::std::shared_ptr<smtk::session::rgg::EditDuct> &)) &smtk::session::rgg::EditDuct::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::EditDuct> (smtk::session::rgg::EditDuct::*)()) &smtk::session::rgg::EditDuct::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::EditDuct> (smtk::session::rgg::EditDuct::*)() const) &smtk::session::rgg::EditDuct::shared_from_this)
    ;
  return instance;
}

#endif
