//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Gallery_h
#define pybind_smtk_task_Gallery_h

#include <pybind11/pybind11.h>

#include "smtk/task/Gallery.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Gallery > pybind11_init_smtk_task_Gallery(py::module &m)
{
  PySharedPtrClass< smtk::task::Gallery > instance(m, "Gallery");
  instance
    .def("add", &smtk::task::Gallery::add, py::arg("workletToBeAdded"), py::arg("makeUnique") = false)
    .def("remove", &smtk::task::Gallery::remove, py::arg("workletToBeRemoved"))
    .def("find", &smtk::task::Gallery::find, py::arg("workletName") )
    .def("worklets", &smtk::task::Gallery::worklets)
    .def("rename", &smtk::task::Gallery::rename, py::arg("workletToBeRenamed"), py::arg("newName"))
    .def("createUniqueName", &smtk::task::Gallery::createUniqueName, py::arg("name") )
    .def("setUniqueNameSeparator", &smtk::task::Gallery::setUniqueNameSeparator, py::arg("newNameSeparator") )
    ;
  return instance;
}

#endif
