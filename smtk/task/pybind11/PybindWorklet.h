//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Worklet_h
#define pybind_smtk_task_Worklet_h

#include <pybind11/pybind11.h>

#include "smtk/task/Worklet.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Worklet, smtk::resource::Component > pybind11_init_smtk_task_Worklet(py::module &m)
{
  PySharedPtrClass< smtk::task::Worklet, smtk::resource::Component > instance(m, "Worklet");
  instance
    .def("schema", &smtk::task::Worklet::schema)
    .def("version", &smtk::task::Worklet::version)
    .def("operationName", &smtk::task::Worklet::operationName)
    .def("description", &smtk::task::Worklet::description)
    .def("configuration", &smtk::task::Worklet::configuration)
    ;
  return instance;
}

#endif
