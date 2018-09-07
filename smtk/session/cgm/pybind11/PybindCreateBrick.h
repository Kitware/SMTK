//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateBrick_h
#define pybind_smtk_session_cgm_operators_CreateBrick_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateBrick.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateBrick > pybind11_init_smtk_session_cgm_CreateBrick(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateBrick > instance(m, "CreateBrick", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateBrick const &>())
    .def("deepcopy", (smtk::session::cgm::CreateBrick & (smtk::session::cgm::CreateBrick::*)(::smtk::session::cgm::CreateBrick const &)) &smtk::session::cgm::CreateBrick::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateBrick> (*)()) &smtk::session::cgm::CreateBrick::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateBrick> (*)(::std::shared_ptr<smtk::session::cgm::CreateBrick> &)) &smtk::session::cgm::CreateBrick::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateBrick::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateBrick> (smtk::session::cgm::CreateBrick::*)() const) &smtk::session::cgm::CreateBrick::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateBrick> (smtk::session::cgm::CreateBrick::*)()) &smtk::session::cgm::CreateBrick::shared_from_this)
    ;
  return instance;
}

#endif
