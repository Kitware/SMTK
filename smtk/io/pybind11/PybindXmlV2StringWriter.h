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

#include "smtk/attribute/System.h"
#include "smtk/io/Logger.h"
#include "smtk/io/XmlStringWriter.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::XmlV2StringWriter, smtk::io::XmlStringWriter > pybind11_init_smtk_io_XmlV2StringWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::XmlV2StringWriter, smtk::io::XmlStringWriter > instance(m, "XmlV2StringWriter");
  instance
    .def(py::init<::smtk::io::XmlV2StringWriter const &>())
    .def(py::init<::smtk::attribute::System const &>())
    .def("convertToString", &smtk::io::XmlV2StringWriter::convertToString, py::arg("logger"), py::arg("no_declaration") = false)
    .def("generateXml", &smtk::io::XmlV2StringWriter::generateXml, py::arg("parent_node"), py::arg("logger"), py::arg("createRoot") = true)
    .def("messageLog", &smtk::io::XmlV2StringWriter::messageLog)
    ;
  return instance;
}

#endif
