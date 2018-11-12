//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_EditCore_h
#define pybind_smtk_session_rgg_operators_EditCore_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/EditCore.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::EditCore, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_EditCore(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::EditCore, smtk::operation::XMLOperation > instance(m, "EditCore");
  instance
    .def(py::init<::smtk::session::rgg::EditCore const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::EditCore & (smtk::session::rgg::EditCore::*)(::smtk::session::rgg::EditCore const &)) &smtk::session::rgg::EditCore::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditCore> (*)()) &smtk::session::rgg::EditCore::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditCore> (*)(::std::shared_ptr<smtk::session::rgg::EditCore> &)) &smtk::session::rgg::EditCore::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::EditCore> (smtk::session::rgg::EditCore::*)()) &smtk::session::rgg::EditCore::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::EditCore> (smtk::session::rgg::EditCore::*)() const) &smtk::session::rgg::EditCore::shared_from_this)
    ;
  return instance;
}

#endif
