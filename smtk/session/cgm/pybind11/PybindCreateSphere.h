//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateSphere_h
#define pybind_smtk_session_cgm_operators_CreateSphere_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateSphere.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateSphere > pybind11_init_smtk_session_cgm_CreateSphere(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateSphere > instance(m, "CreateSphere", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateSphere const &>())
    .def("deepcopy", (smtk::session::cgm::CreateSphere & (smtk::session::cgm::CreateSphere::*)(::smtk::session::cgm::CreateSphere const &)) &smtk::session::cgm::CreateSphere::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateSphere> (*)()) &smtk::session::cgm::CreateSphere::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateSphere> (*)(::std::shared_ptr<smtk::session::cgm::CreateSphere> &)) &smtk::session::cgm::CreateSphere::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateSphere::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateSphere> (smtk::session::cgm::CreateSphere::*)() const) &smtk::session::cgm::CreateSphere::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateSphere> (smtk::session::cgm::CreateSphere::*)()) &smtk::session::cgm::CreateSphere::shared_from_this)
    ;
  return instance;
}

#endif
