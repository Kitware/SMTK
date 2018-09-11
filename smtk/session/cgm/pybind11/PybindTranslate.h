//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Translate_h
#define pybind_smtk_session_cgm_operators_Translate_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Translate.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Translate > pybind11_init_smtk_session_cgm_Translate(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Translate > instance(m, "Translate", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Translate const &>())
    .def("deepcopy", (smtk::session::cgm::Translate & (smtk::session::cgm::Translate::*)(::smtk::session::cgm::Translate const &)) &smtk::session::cgm::Translate::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Translate> (*)()) &smtk::session::cgm::Translate::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Translate> (*)(::std::shared_ptr<smtk::session::cgm::Translate> &)) &smtk::session::cgm::Translate::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Translate::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Translate> (smtk::session::cgm::Translate::*)() const) &smtk::session::cgm::Translate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Translate> (smtk::session::cgm::Translate::*)()) &smtk::session::cgm::Translate::shared_from_this)
    ;
  return instance;
}

#endif
