//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_FileLocation_h
#define pybind_smtk_common_FileLocation_h

#include <pybind11/pybind11.h>

#include "smtk/common/FileLocation.h"

namespace py = pybind11;

py::class_< smtk::common::FileLocation > pybind11_init_smtk_common_FileLocation(py::module &m)
{
  py::class_< smtk::common::FileLocation > instance(m, "FileLocation");
  instance
    .def(py::init<>())
    .def(py::init<::std::string const &, ::std::string const &>())
    .def(py::init<::smtk::common::FileLocation const &>())
    .def("deepcopy", (smtk::common::FileLocation & (smtk::common::FileLocation::*)(::smtk::common::FileLocation const &)) &smtk::common::FileLocation::operator=)
    .def("__eq__", (bool (smtk::common::FileLocation::*)(::smtk::common::FileLocation const &) const) &smtk::common::FileLocation::operator==)
    .def("__eq__", (bool (smtk::common::FileLocation::*)(::std::string const &) const) &smtk::common::FileLocation::operator==)
    .def("absolutePath", &smtk::common::FileLocation::absolutePath)
    .def("relativePath", &smtk::common::FileLocation::relativePath)
    .def("referencePath", &smtk::common::FileLocation::referencePath)
    .def("empty", &smtk::common::FileLocation::empty)
    .def("clear", &smtk::common::FileLocation::clear)
    ;
  return instance;
}

#endif
