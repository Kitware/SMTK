//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_XmlV2StringWriter_h
#define pybind_smtk_io_XmlV2StringWriter_h

#include <pybind11/pybind11.h>

#include "smtk/io/XmlV2StringWriter.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::XmlV2StringWriter > pybind11_init_smtk_io_XmlV2StringWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::XmlV2StringWriter > instance(m, "XmlV2StringWriter");
  instance
    .def(py::init<::smtk::io::XmlV2StringWriter const &>())
    .def(py::init<::smtk::attribute::System const &>())
    .def("convertToString", &smtk::io::XmlV2StringWriter::convertToString, py::arg("logger"), py::arg("no_declaration") = false)
    .def("generateXml", &smtk::io::XmlV2StringWriter::generateXml, py::arg("parent_node"), py::arg("logger"), py::arg("createRoot") = true)
    .def("includeDefinitions", &smtk::io::XmlV2StringWriter::includeDefinitions, py::arg("val"))
    .def("includeInstances", &smtk::io::XmlV2StringWriter::includeInstances, py::arg("val"))
    .def("includeModelInformation", &smtk::io::XmlV2StringWriter::includeModelInformation, py::arg("val"))
    .def("includeViews", &smtk::io::XmlV2StringWriter::includeViews, py::arg("val"))
    .def("messageLog", &smtk::io::XmlV2StringWriter::messageLog)
    ;
  return instance;
}

#endif
