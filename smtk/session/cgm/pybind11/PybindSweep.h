//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Sweep_h
#define pybind_smtk_session_cgm_operators_Sweep_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Sweep.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Sweep > pybind11_init_smtk_session_cgm_Sweep(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Sweep > instance(m, "Sweep", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Sweep const &>())
    .def("deepcopy", (smtk::session::cgm::Sweep & (smtk::session::cgm::Sweep::*)(::smtk::session::cgm::Sweep const &)) &smtk::session::cgm::Sweep::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Sweep> (*)()) &smtk::session::cgm::Sweep::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Sweep> (*)(::std::shared_ptr<smtk::session::cgm::Sweep> &)) &smtk::session::cgm::Sweep::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Sweep::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Sweep> (smtk::session::cgm::Sweep::*)() const) &smtk::session::cgm::Sweep::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Sweep> (smtk::session::cgm::Sweep::*)()) &smtk::session::cgm::Sweep::shared_from_this)
    ;
  return instance;
}

#endif
