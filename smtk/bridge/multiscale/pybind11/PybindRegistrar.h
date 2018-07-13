//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_Registrar_h
#define pybind_smtk_bridge_multiscale_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/Registrar.h"

namespace py = pybind11;

py::class_< smtk::bridge::multiscale::Registrar > pybind11_init_smtk_bridge_multiscale_Registrar(py::module &m)
{
  py::class_< smtk::bridge::multiscale::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::bridge::multiscale::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::bridge::multiscale::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::bridge::multiscale::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::bridge::multiscale::Registrar::unregisterFrom)
    ;
  return instance;
}

#endif
