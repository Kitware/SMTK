//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateEdge_h
#define pybind_smtk_session_cgm_operators_CreateEdge_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateEdge.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateEdge > pybind11_init_smtk_session_cgm_CreateEdge(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateEdge > instance(m, "CreateEdge", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateEdge const &>())
    .def("deepcopy", (smtk::session::cgm::CreateEdge & (smtk::session::cgm::CreateEdge::*)(::smtk::session::cgm::CreateEdge const &)) &smtk::session::cgm::CreateEdge::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateEdge> (*)()) &smtk::session::cgm::CreateEdge::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateEdge> (*)(::std::shared_ptr<smtk::session::cgm::CreateEdge> &)) &smtk::session::cgm::CreateEdge::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateEdge::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateEdge> (smtk::session::cgm::CreateEdge::*)() const) &smtk::session::cgm::CreateEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateEdge> (smtk::session::cgm::CreateEdge::*)()) &smtk::session::cgm::CreateEdge::shared_from_this)
    ;
  return instance;
}

#endif
