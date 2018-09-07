//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Scale_h
#define pybind_smtk_session_cgm_operators_Scale_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Scale.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Scale > pybind11_init_smtk_session_cgm_Scale(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Scale > instance(m, "Scale", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Scale const &>())
    .def("deepcopy", (smtk::session::cgm::Scale & (smtk::session::cgm::Scale::*)(::smtk::session::cgm::Scale const &)) &smtk::session::cgm::Scale::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Scale> (*)()) &smtk::session::cgm::Scale::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Scale> (*)(::std::shared_ptr<smtk::session::cgm::Scale> &)) &smtk::session::cgm::Scale::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Scale::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Scale> (smtk::session::cgm::Scale::*)() const) &smtk::session::cgm::Scale::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Scale> (smtk::session::cgm::Scale::*)()) &smtk::session::cgm::Scale::shared_from_this)
    ;
  return instance;
}

#endif
