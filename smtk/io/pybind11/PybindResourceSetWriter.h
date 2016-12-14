//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ResourceSetWriter_h
#define pybind_smtk_io_ResourceSetWriter_h

#include <pybind11/pybind11.h>

#include "smtk/io/ResourceSetWriter.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ResourceSetWriter > pybind11_init_smtk_io_ResourceSetWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::ResourceSetWriter > instance(m, "ResourceSetWriter");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ResourceSetWriter const &>())
    .def("deepcopy", (smtk::io::ResourceSetWriter & (smtk::io::ResourceSetWriter::*)(::smtk::io::ResourceSetWriter const &)) &smtk::io::ResourceSetWriter::operator=);
  py::enum_<smtk::io::ResourceSetWriter::LinkedFilesOption>(instance, "LinkedFilesOption")
    .value("SKIP_LINKED_FILES", smtk::io::ResourceSetWriter::LinkedFilesOption::SKIP_LINKED_FILES)
    .value("EXPAND_LINKED_FILES", smtk::io::ResourceSetWriter::LinkedFilesOption::EXPAND_LINKED_FILES)
    .value("WRITE_LINKED_FILES", smtk::io::ResourceSetWriter::LinkedFilesOption::WRITE_LINKED_FILES)
    .export_values();
  instance
    .def("writeFile", &smtk::io::ResourceSetWriter::writeFile, py::arg("filename"), py::arg("resources"), py::arg("logger"), py::arg("option") = ::smtk::io::ResourceSetWriter::LinkedFilesOption::WRITE_LINKED_FILES)
    .def("writeString", &smtk::io::ResourceSetWriter::writeString, py::arg("content"), py::arg("resources"), py::arg("logger"), py::arg("option") = ::smtk::io::ResourceSetWriter::LinkedFilesOption::WRITE_LINKED_FILES)
    ;
  return instance;
}

#endif
