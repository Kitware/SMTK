//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateVertex_h
#define pybind_smtk_session_cgm_operators_CreateVertex_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateVertex.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateVertex > pybind11_init_smtk_session_cgm_CreateVertex(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateVertex > instance(m, "CreateVertex", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateVertex const &>())
    .def("deepcopy", (smtk::session::cgm::CreateVertex & (smtk::session::cgm::CreateVertex::*)(::smtk::session::cgm::CreateVertex const &)) &smtk::session::cgm::CreateVertex::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateVertex> (*)()) &smtk::session::cgm::CreateVertex::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateVertex> (*)(::std::shared_ptr<smtk::session::cgm::CreateVertex> &)) &smtk::session::cgm::CreateVertex::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateVertex::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateVertex> (smtk::session::cgm::CreateVertex::*)() const) &smtk::session::cgm::CreateVertex::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateVertex> (smtk::session::cgm::CreateVertex::*)()) &smtk::session::cgm::CreateVertex::shared_from_this)
    ;
  return instance;
}

#endif
