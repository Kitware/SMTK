//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_PathsHelperUnix_h
#define pybind_smtk_common_PathsHelperUnix_h

#include <pybind11/pybind11.h>

#include "smtk/common/PathsHelperUnix.h"

namespace py = pybind11;

py::class_< smtk::common::PathsHelperUnix > pybind11_init_smtk_common_PathsHelperUnix(py::module &m)
{
  py::class_< smtk::common::PathsHelperUnix > instance(m, "PathsHelperUnix");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::PathsHelperUnix const &>())
    .def("deepcopy", (smtk::common::PathsHelperUnix & (smtk::common::PathsHelperUnix::*)(::smtk::common::PathsHelperUnix const &)) &smtk::common::PathsHelperUnix::operator=)
    .def_static("AddSplitPaths", &smtk::common::PathsHelperUnix::AddSplitPaths, py::arg("splitPaths"), py::arg("envVar"))
    ;
  return instance;
}

#endif
