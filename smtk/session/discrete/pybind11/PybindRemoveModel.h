//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_RemoveModel_h
#define pybind_smtk_session_discrete_operators_RemoveModel_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/RemoveModel.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::RemoveModel, smtk::operation::Operation > pybind11_init_smtk_session_discrete_RemoveModel(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::RemoveModel, smtk::operation::Operation > instance(m, "RemoveModel");
  instance
    .def(py::init<::smtk::session::discrete::RemoveModel const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::discrete::RemoveModel & (smtk::session::discrete::RemoveModel::*)(::smtk::session::discrete::RemoveModel const &)) &smtk::session::discrete::RemoveModel::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::RemoveModel> (*)()) &smtk::session::discrete::RemoveModel::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::RemoveModel> (*)(::std::shared_ptr<smtk::session::discrete::RemoveModel> &)) &smtk::session::discrete::RemoveModel::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::RemoveModel> (smtk::session::discrete::RemoveModel::*)()) &smtk::session::discrete::RemoveModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::RemoveModel> (smtk::session::discrete::RemoveModel::*)() const) &smtk::session::discrete::RemoveModel::shared_from_this)
    .def("name", &smtk::session::discrete::RemoveModel::name)
    .def("ableToOperate", &smtk::session::discrete::RemoveModel::ableToOperate)
    ;
  return instance;
}

#endif
