//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Registrar_h
#define pybind_smtk_job_Registrar_h

#include "smtk/job/Manager.h"
#include "smtk/job/Registrar.h"

namespace py = pybind11;

inline py::class_<smtk::job::Registrar> pybind11_init_smtk_job_Registrar(py::module &m)
{
  py::class_<smtk::job::Registrar> instance(m, "Registrar");
  instance
    .def_static("registerTo", (void (*)(std::shared_ptr<smtk::common::Managers> const&)) &smtk::job::Registrar::registerTo, py::arg("application_context"))
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<smtk::common::Managers> const&)) &smtk::job::Registrar::unregisterFrom, py::arg("application_context"))

    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::job::Registrar::registerTo, py::arg("resource_manager"))
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::resource::Manager> const &)) &smtk::job::Registrar::unregisterFrom, py::arg("resource_manager"))

    .def_static("registerTo", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::job::Registrar::registerTo, py::arg("operation_manager"))
    .def_static("unregisterFrom", (void (*)(std::shared_ptr<::smtk::operation::Manager> const &)) &smtk::job::Registrar::unregisterFrom, py::arg("operation_manager"))
    ;
  return instance;
}

#endif
