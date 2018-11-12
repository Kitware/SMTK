//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_CreateModel_h
#define pybind_smtk_session_rgg_operators_CreateModel_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/CreateModel.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::CreateModel, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_CreateModel(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::CreateModel, smtk::operation::XMLOperation > instance(m, "CreateModel");
  instance
    .def(py::init<::smtk::session::rgg::CreateModel const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::CreateModel & (smtk::session::rgg::CreateModel::*)(::smtk::session::rgg::CreateModel const &)) &smtk::session::rgg::CreateModel::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateModel> (*)()) &smtk::session::rgg::CreateModel::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::CreateModel> (*)(::std::shared_ptr<smtk::session::rgg::CreateModel> &)) &smtk::session::rgg::CreateModel::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::CreateModel> (smtk::session::rgg::CreateModel::*)()) &smtk::session::rgg::CreateModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::CreateModel> (smtk::session::rgg::CreateModel::*)() const) &smtk::session::rgg::CreateModel::shared_from_this)
    ;
  return instance;
}

#endif
