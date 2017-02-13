//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___XmlStringWriter_h
#define pybind___XmlStringWriter_h

#include <pybind11/pybind11.h>

#include "smtk/io/XmlStringWriter.h"

#include "smtk/attribute/System.h"
#include "smtk/io/Logger.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::XmlStringWriter > pybind11_init_smtk_io_XmlStringWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::XmlStringWriter > instance(m, "XmlStringWriter");
  instance
    .def("className", &smtk::io::XmlStringWriter::className)
    .def("convertToString", &smtk::io::XmlStringWriter::convertToString, py::arg("logger"), py::arg("no_declaration") = false)
    .def("fileVersion", &smtk::io::XmlStringWriter::fileVersion)
    .def("generateXml", &smtk::io::XmlStringWriter::generateXml, py::arg("parent_node"), py::arg("logger"), py::arg("createRoot") = true)
    .def("includeDefinitions", &smtk::io::XmlStringWriter::includeDefinitions, py::arg("val"))
    .def("includeInstances", &smtk::io::XmlStringWriter::includeInstances, py::arg("val"))
    .def("includeModelInformation", &smtk::io::XmlStringWriter::includeModelInformation, py::arg("val"))
    .def("includeViews", &smtk::io::XmlStringWriter::includeViews, py::arg("val"))
    ;
  return instance;
}

#endif
