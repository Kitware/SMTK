//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_PathsHelperWindows_h
#define pybind_smtk_common_PathsHelperWindows_h

#include <pybind11/pybind11.h>

#include "smtk/common/PathsHelperWindows.h"

namespace py = pybind11;

py::class_< smtk::common::PathsHelperWindows > pybind11_init_smtk_common_PathsHelperWindows(py::module &m)
{
  py::class_< smtk::common::PathsHelperWindows > instance(m, "PathsHelperWindows");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::PathsHelperWindows const &>())
    .def("deepcopy", (smtk::common::PathsHelperWindows & (smtk::common::PathsHelperWindows::*)(::smtk::common::PathsHelperWindows const &)) &smtk::common::PathsHelperWindows::operator=)
    .def_static("AddSplitPaths", &smtk::common::PathsHelperWindows::AddSplitPaths, py::arg("splitPaths"), py::arg("envVar"))
    ;
  return instance;
}

#endif
