//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateFace_h
#define pybind_smtk_session_cgm_operators_CreateFace_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateFace.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateFace > pybind11_init_smtk_session_cgm_CreateFace(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateFace > instance(m, "CreateFace", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateFace const &>())
    .def("deepcopy", (smtk::session::cgm::CreateFace & (smtk::session::cgm::CreateFace::*)(::smtk::session::cgm::CreateFace const &)) &smtk::session::cgm::CreateFace::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateFace> (*)()) &smtk::session::cgm::CreateFace::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateFace> (*)(::std::shared_ptr<smtk::session::cgm::CreateFace> &)) &smtk::session::cgm::CreateFace::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateFace::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateFace> (smtk::session::cgm::CreateFace::*)() const) &smtk::session::cgm::CreateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateFace> (smtk::session::cgm::CreateFace::*)()) &smtk::session::cgm::CreateFace::shared_from_this)
    ;
  return instance;
}

#endif
