//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_PortData_h
#define pybind_smtk_task_PortData_h

#include <pybind11/pybind11.h>

#include "smtk/task/PortData.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::PortData > pybind11_init_smtk_task_PortData(py::module &m)
{
  PySharedPtrClass< smtk::task::PortData > instance(m, "PortData");
  instance
    .def("typeName", &smtk::task::PortData::typeName)
    .def_static("create", (std::shared_ptr<smtk::task::PortData> (*)()) &smtk::task::PortData::create)
    .def("merge", [](smtk::task::PortData& self, const std::shared_ptr<smtk::task::PortData>& other)
      {
        return self.merge(other.get());
      })
    ;
  return instance;
}

#endif
