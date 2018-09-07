//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreateCylinder_h
#define pybind_smtk_session_cgm_operators_CreateCylinder_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreateCylinder.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreateCylinder > pybind11_init_smtk_session_cgm_CreateCylinder(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreateCylinder > instance(m, "CreateCylinder", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreateCylinder const &>())
    .def("deepcopy", (smtk::session::cgm::CreateCylinder & (smtk::session::cgm::CreateCylinder::*)(::smtk::session::cgm::CreateCylinder const &)) &smtk::session::cgm::CreateCylinder::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateCylinder> (*)()) &smtk::session::cgm::CreateCylinder::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreateCylinder> (*)(::std::shared_ptr<smtk::session::cgm::CreateCylinder> &)) &smtk::session::cgm::CreateCylinder::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreateCylinder::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreateCylinder> (smtk::session::cgm::CreateCylinder::*)() const) &smtk::session::cgm::CreateCylinder::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreateCylinder> (smtk::session::cgm::CreateCylinder::*)()) &smtk::session::cgm::CreateCylinder::shared_from_this)
    ;
  return instance;
}

#endif
