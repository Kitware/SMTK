//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Manager_h
#define pybind_smtk_task_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/task/Manager.h"

#include "smtk/task/Active.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Manager > pybind11_init_smtk_task_Manager(py::module &m)
{
  PySharedPtrClass< smtk::task::Manager > instance(m, "Manager");
  instance
    .def("active", (smtk::task::Active & (smtk::task::Manager::*)()) &smtk::task::Manager::active)
    .def("active", (smtk::task::Active const & (smtk::task::Manager::*)() const) &smtk::task::Manager::active)
    .def_static("create", (std::shared_ptr<smtk::task::Manager> (*)()) &smtk::task::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::task::Manager> (*)(::std::shared_ptr<smtk::task::Manager> &)) &smtk::task::Manager::create, py::arg("ref"))
    .def("adaptorInstances", (smtk::task::Manager::AdaptorInstances & (smtk::task::Manager::*)()) &smtk::task::Manager::adaptorInstances)
    .def("adaptorInstances", (smtk::task::Manager::AdaptorInstances const & (smtk::task::Manager::*)() const) &smtk::task::Manager::adaptorInstances)
    .def("taskInstances", (smtk::task::Manager::TaskInstances & (smtk::task::Manager::*)()) &smtk::task::Manager::taskInstances)
    .def("taskInstances", (smtk::task::Manager::TaskInstances const & (smtk::task::Manager::*)() const) &smtk::task::Manager::taskInstances)
    .def("managers", &smtk::task::Manager::managers)
    .def("setManagers", &smtk::task::Manager::setManagers, py::arg("managers"))
    .def("typeName", &smtk::task::Manager::typeName)
    .def_readonly_static("type_name", &smtk::task::Manager::type_name)
    ;
  return instance;
}

#endif
