//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_CreateAssembly_h
#define pybind_smtk_session_rgg_operators_CreateAssembly_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/CreateAssembly.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::CreateAssembly, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_CreateAssembly(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::CreateAssembly, smtk::operation::XMLOperation > instance(m, "CreateAssembly");
  instance
    .def(py::init<::smtk::session::rgg::CreateAssembly const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::CreateAssembly & (smtk::session::rgg::CreateAssembly::*)(::smtk::session::rgg::CreateAssembly const &)) &smtk::session::rgg::CreateAssembly::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateAssembly> (*)()) &smtk::session::rgg::CreateAssembly::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateAssembly> (*)(::std::shared_ptr<smtk::session::rgg::CreateAssembly> &)) &smtk::session::rgg::CreateAssembly::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::CreateAssembly> (smtk::session::rgg::CreateAssembly::*)()) &smtk::session::rgg::CreateAssembly::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::CreateAssembly> (smtk::session::rgg::CreateAssembly::*)() const) &smtk::session::rgg::CreateAssembly::shared_from_this)
    ;
  return instance;
}

#endif
