//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_multiscale_operators_Revolve_h
#define pybind_smtk_session_multiscale_operators_Revolve_h

#include <pybind11/pybind11.h>

#include "smtk/session/multiscale/operators/Revolve.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::multiscale::Revolve, smtk::operation::XMLOperation > pybind11_init_smtk_session_multiscale_Revolve(py::module &m)
{
  PySharedPtrClass< smtk::session::multiscale::Revolve, smtk::operation::XMLOperation > instance(m, "Revolve");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::multiscale::Revolve const &>())
    .def("deepcopy", (smtk::session::multiscale::Revolve & (smtk::session::multiscale::Revolve::*)(::smtk::session::multiscale::Revolve const &)) &smtk::session::multiscale::Revolve::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Revolve> (*)()) &smtk::session::multiscale::Revolve::create)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Revolve> (*)(::std::shared_ptr<smtk::session::multiscale::Revolve> &)) &smtk::session::multiscale::Revolve::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::multiscale::Revolve> (smtk::session::multiscale::Revolve::*)() const) &smtk::session::multiscale::Revolve::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::multiscale::Revolve> (smtk::session::multiscale::Revolve::*)()) &smtk::session::multiscale::Revolve::shared_from_this)
    ;
  return instance;
}

#endif
