//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__build_smtk_smtk_common_Version_h
#define pybind__build_smtk_smtk_common_Version_h

#include <pybind11/pybind11.h>

#include "smtk/common/Version.h"

namespace py = pybind11;

py::class_< smtk::common::Version > pybind11_init_smtk_common_Version(py::module &m)
{
  py::class_< smtk::common::Version > instance(m, "Version", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::Version const &>())
    .def("deepcopy", (smtk::common::Version & (smtk::common::Version::*)(::smtk::common::Version const &)) &smtk::common::Version::operator=)
    // Note that major() and minor() are posix keywords, and castxml outputs some odd syntax on linux (gnu_dev_major).
    // So manually define major() and minor() here with lambda expressions.
    .def_static("major", []{return smtk::common::Version::major();})
    .def_static("minor", []{return smtk::common::Version::minor();})
    .def_static("patch", &smtk::common::Version::patch)
    .def_static("number", &smtk::common::Version::number)
    .def_static("combined", (int (*)()) &smtk::common::Version::combined)
    .def_static("combined", (int (*)(int, int, int)) &smtk::common::Version::combined, py::arg("maj"), py::arg("min"), py::arg("pat"))
    ;
  return instance;
}

#endif
