//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_PathsHelperMacOSX_h
#define pybind_smtk_common_PathsHelperMacOSX_h

#include <pybind11/pybind11.h>

#include "smtk/common/PathsHelperMacOSX.h"

namespace py = pybind11;

py::class_< smtk::common::PathsHelperMacOSX > pybind11_init_smtk_common_PathsHelperMacOSX(py::module &m)
{
  py::class_< smtk::common::PathsHelperMacOSX > instance(m, "PathsHelperMacOSX");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::PathsHelperMacOSX const &>())
    .def("deepcopy", (smtk::common::PathsHelperMacOSX & (smtk::common::PathsHelperMacOSX::*)(::smtk::common::PathsHelperMacOSX const &)) &smtk::common::PathsHelperMacOSX::operator=)
    ;
  return instance;
}

#endif
