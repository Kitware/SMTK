//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ResourceSetReader_h
#define pybind_smtk_io_ResourceSetReader_h

#include <pybind11/pybind11.h>

#include "smtk/io/ResourceSetReader.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ResourceSetReader > pybind11_init_smtk_io_ResourceSetReader(py::module &m)
{
  PySharedPtrClass< smtk::io::ResourceSetReader > instance(m, "ResourceSetReader");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ResourceSetReader const &>())
    .def("deepcopy", (smtk::io::ResourceSetReader & (smtk::io::ResourceSetReader::*)(::smtk::io::ResourceSetReader const &)) &smtk::io::ResourceSetReader::operator=)
    .def("readFile", &smtk::io::ResourceSetReader::readFile, py::arg("filename"), py::arg("resources"), py::arg("logger"), py::arg("loadLinkedFiles") = true)
    .def("readString", &smtk::io::ResourceSetReader::readString, py::arg("content"), py::arg("resources"), py::arg("logger"), py::arg("loadLinkedFiles") = true, py::arg("resourceMap") = __null)
    ;
  return instance;
}

#endif
