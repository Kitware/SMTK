//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_Registrar_h
#define pybind_smtk_markup_Registrar_h

#include <pybind11/pybind11.h>

#include "smtk/markup/Registrar.h"

#include "smtk/common/Managers.h"
#include "smtk/resource/Manager.h"
#include "smtk/operation/Manager.h"
#include "smtk/view/Manager.h"

namespace py = pybind11;

inline py::class_< smtk::markup::Registrar > pybind11_init_smtk_markup_Registrar(py::module &m)
{
  py::class_< smtk::markup::Registrar > instance(m, "Registrar");
  instance
    .def(py::init<>())
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::common::Managers> const &)) &smtk::markup::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::common::Managers> const &)) &smtk::markup::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::markup::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::markup::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::markup::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::markup::Registrar::unregisterFrom)
    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::view::Manager> const &)) &smtk::markup::Registrar::registerTo)
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::view::Manager> const &)) &smtk::markup::Registrar::unregisterFrom)
    ;
  return instance;
}

#endif
