//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_CreatePrism_h
#define pybind_smtk_session_cgm_operators_CreatePrism_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/CreatePrism.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::CreatePrism > pybind11_init_smtk_session_cgm_CreatePrism(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::CreatePrism > instance(m, "CreatePrism", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::CreatePrism const &>())
    .def("deepcopy", (smtk::session::cgm::CreatePrism & (smtk::session::cgm::CreatePrism::*)(::smtk::session::cgm::CreatePrism const &)) &smtk::session::cgm::CreatePrism::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreatePrism> (*)()) &smtk::session::cgm::CreatePrism::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::CreatePrism> (*)(::std::shared_ptr<smtk::session::cgm::CreatePrism> &)) &smtk::session::cgm::CreatePrism::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::CreatePrism::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::CreatePrism> (smtk::session::cgm::CreatePrism::*)() const) &smtk::session::cgm::CreatePrism::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::CreatePrism> (smtk::session::cgm::CreatePrism::*)()) &smtk::session::cgm::CreatePrism::shared_from_this)
    ;
  return instance;
}

#endif
