//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_EditAssembly_h
#define pybind_smtk_session_rgg_operators_EditAssembly_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/EditAssembly.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::EditAssembly, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_EditAssembly(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::EditAssembly, smtk::operation::XMLOperation > instance(m, "EditAssembly");
  instance
    .def(py::init<::smtk::session::rgg::EditAssembly const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::EditAssembly & (smtk::session::rgg::EditAssembly::*)(::smtk::session::rgg::EditAssembly const &)) &smtk::session::rgg::EditAssembly::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditAssembly> (*)()) &smtk::session::rgg::EditAssembly::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditAssembly> (*)(::std::shared_ptr<smtk::session::rgg::EditAssembly> &)) &smtk::session::rgg::EditAssembly::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::EditAssembly> (smtk::session::rgg::EditAssembly::*)()) &smtk::session::rgg::EditAssembly::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::EditAssembly> (smtk::session::rgg::EditAssembly::*)() const) &smtk::session::rgg::EditAssembly::shared_from_this)
    ;
  return instance;
}

#endif
