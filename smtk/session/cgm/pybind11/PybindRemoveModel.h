//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_RemoveModel_h
#define pybind_smtk_session_cgm_operators_RemoveModel_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/RemoveModel.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::RemoveModel > pybind11_init_smtk_session_cgm_RemoveModel(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::RemoveModel > instance(m, "RemoveModel", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::RemoveModel const &>())
    .def("deepcopy", (smtk::session::cgm::RemoveModel & (smtk::session::cgm::RemoveModel::*)(::smtk::session::cgm::RemoveModel const &)) &smtk::session::cgm::RemoveModel::operator=)
    .def("ableToOperate", &smtk::session::cgm::RemoveModel::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::RemoveModel> (*)()) &smtk::session::cgm::RemoveModel::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::RemoveModel> (*)(::std::shared_ptr<smtk::session::cgm::RemoveModel> &)) &smtk::session::cgm::RemoveModel::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::RemoveModel::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::RemoveModel> (smtk::session::cgm::RemoveModel::*)() const) &smtk::session::cgm::RemoveModel::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::RemoveModel> (smtk::session::cgm::RemoveModel::*)()) &smtk::session::cgm::RemoveModel::shared_from_this)
    ;
  return instance;
}

#endif
