//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_Registrar_h
#define pybind_smtk_session_discrete_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/Registrar.h"

namespace py = pybind11;

py::class_< smtk::session::discrete::Registrar > pybind11_init_smtk_session_discrete_Registrar(py::module &m)
{
  py::class_< smtk::session::discrete::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::session::discrete::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::session::discrete::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::session::discrete::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::session::discrete::Registrar::unregisterFrom)
    ;
  return instance;
}

#endif
