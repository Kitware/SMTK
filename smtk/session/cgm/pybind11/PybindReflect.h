//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Reflect_h
#define pybind_smtk_session_cgm_operators_Reflect_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Reflect.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Reflect > pybind11_init_smtk_session_cgm_Reflect(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Reflect > instance(m, "Reflect", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Reflect const &>())
    .def("deepcopy", (smtk::session::cgm::Reflect & (smtk::session::cgm::Reflect::*)(::smtk::session::cgm::Reflect const &)) &smtk::session::cgm::Reflect::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Reflect> (*)()) &smtk::session::cgm::Reflect::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Reflect> (*)(::std::shared_ptr<smtk::session::cgm::Reflect> &)) &smtk::session::cgm::Reflect::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Reflect::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Reflect> (smtk::session::cgm::Reflect::*)() const) &smtk::session::cgm::Reflect::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Reflect> (smtk::session::cgm::Reflect::*)()) &smtk::session::cgm::Reflect::shared_from_this)
    ;
  return instance;
}

#endif
