//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_StringUtil_h
#define pybind_smtk_common_StringUtil_h

#include <pybind11/pybind11.h>

#include "smtk/common/StringUtil.h"

namespace py = pybind11;

py::class_< smtk::common::StringUtil > pybind11_init_smtk_common_StringUtil(py::module &m)
{
  py::class_< smtk::common::StringUtil > instance(m, "StringUtil");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::StringUtil const &>())
    .def("deepcopy", (smtk::common::StringUtil & (smtk::common::StringUtil::*)(::smtk::common::StringUtil const &)) &smtk::common::StringUtil::operator=)
    .def_static("trim", &smtk::common::StringUtil::trim, py::arg("s"))
    .def_static("trimLeft", &smtk::common::StringUtil::trimLeft, py::arg("s"))
    .def_static("trimRight", &smtk::common::StringUtil::trimRight, py::arg("s"))
    .def_static("lower", &smtk::common::StringUtil::lower, py::arg("s"))
    .def_static("upper", &smtk::common::StringUtil::upper, py::arg("s"))
    .def_static("split", &smtk::common::StringUtil::split, py::arg("s"), py::arg("sep"), py::arg("omitEmpty"), py::arg("trim"))
    ;
  return instance;
}

#endif
