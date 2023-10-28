//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Paths_h
#define pybind_smtk_common_Paths_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/common/Paths.h"

namespace py = pybind11;

inline py::class_< smtk::common::Paths > pybind11_init_smtk_common_Paths(py::module &m)
{
  py::class_< smtk::common::Paths > instance(m, "Paths");
  instance
    .def(py::init<>())
    .def(py::init<::std::string const &>())
    .def(py::init<::smtk::common::Paths const &>())
    .def("deepcopy", (smtk::common::Paths & (smtk::common::Paths::*)(::smtk::common::Paths const &)) &smtk::common::Paths::operator=)
    .def_static("currentDirectory", &smtk::common::Paths::currentDirectory)
    .def_static("directoryExists", &smtk::common::Paths::directoryExists, py::arg("path"))
    .def_static("pathToThisLibrary", &smtk::common::Paths::pathToThisLibrary)
    .def_static("pruneInvalidDirectories", &smtk::common::Paths::pruneInvalidDirectories, py::arg("src"))
    .def("executableDirectory", &smtk::common::Paths::executableDirectory)
    .def("toplevelDirectory", &smtk::common::Paths::toplevelDirectory)
    .def("bundleDirectory", &smtk::common::Paths::bundleDirectory)
    .def("workerSearchPaths", &smtk::common::Paths::workerSearchPaths, py::arg("pruneInvalid") = true)
    .def("toplevelDirectoryConfigured", &smtk::common::Paths::toplevelDirectoryConfigured)
    .def("forceUpdate", &smtk::common::Paths::forceUpdate)
    ;
  return instance;
}

#endif
