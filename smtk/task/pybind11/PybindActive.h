//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Active_h
#define pybind_smtk_task_Active_h

#include <pybind11/pybind11.h>

#include "smtk/task/Active.h"

#include "smtk/task/Instances.h"
#include "smtk/task/Task.h"

namespace py = pybind11;

inline py::class_< smtk::task::Active > pybind11_init_smtk_task_Active(py::module &m)
{
  py::class_< smtk::task::Active > instance(m, "Active");
  instance
    .def(py::init<::smtk::task::Instances *>())
    .def("observers", (smtk::task::Active::Observers & (smtk::task::Active::*)()) &smtk::task::Active::observers)
    .def("observers", (smtk::task::Active::Observers const & (smtk::task::Active::*)() const) &smtk::task::Active::observers)
    .def("switchTo", &smtk::task::Active::switchTo, py::arg("arg0"))
    .def("task", &smtk::task::Active::task)
    .def("typeName", &smtk::task::Active::typeName)
    ;
  return instance;
}

#endif
