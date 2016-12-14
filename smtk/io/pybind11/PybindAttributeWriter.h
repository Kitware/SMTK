//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_AttributeWriter_h
#define pybind_smtk_io_AttributeWriter_h

#include <pybind11/pybind11.h>

#include "smtk/io/AttributeWriter.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::AttributeWriter > pybind11_init_smtk_io_AttributeWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::AttributeWriter > instance(m, "AttributeWriter");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::AttributeWriter const &>())
    .def("deepcopy", (smtk::io::AttributeWriter & (smtk::io::AttributeWriter::*)(::smtk::io::AttributeWriter const &)) &smtk::io::AttributeWriter::operator=)
    .def("write", &smtk::io::AttributeWriter::write, py::arg("system"), py::arg("filename"), py::arg("logger"))
    .def("writeContents", &smtk::io::AttributeWriter::writeContents, py::arg("system"), py::arg("filecontents"), py::arg("logger"), py::arg("no_declaration") = false)
    .def("includeDefinitions", &smtk::io::AttributeWriter::includeDefinitions, py::arg("val"))
    .def("includeInstances", &smtk::io::AttributeWriter::includeInstances, py::arg("val"))
    .def("includeModelInformation", &smtk::io::AttributeWriter::includeModelInformation, py::arg("val"))
    .def("includeViews", &smtk::io::AttributeWriter::includeViews, py::arg("val"))
    ;
  return instance;
}

#endif
