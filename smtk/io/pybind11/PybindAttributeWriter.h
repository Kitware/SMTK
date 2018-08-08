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

#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::AttributeWriter > pybind11_init_smtk_io_AttributeWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::AttributeWriter > instance(m, "AttributeWriter");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::AttributeWriter const &>())
    .def("deepcopy", (smtk::io::AttributeWriter & (smtk::io::AttributeWriter::*)(::smtk::io::AttributeWriter const &)) &smtk::io::AttributeWriter::operator=)
    .def("setFileVersion", &smtk::io::AttributeWriter::setFileVersion, py::arg("version"))
    .def("setMaxFileVersion", &smtk::io::AttributeWriter::setMaxFileVersion)
    .def("fileVersion", &smtk::io::AttributeWriter::fileVersion)
    .def("write", &smtk::io::AttributeWriter::write, py::arg("resource"), py::arg("filename"), py::arg("logger"))
    // As per python convention, all strings passed to functions are immutable (see pybind11 FAQ).
//    .def("writeContents", &smtk::io::AttributeWriter::writeContents, py::arg("resource"), py::arg("filecontents"), py::arg("logger"), py::arg("no_declaration") = false)
    .def("writeContents", [](smtk::io::AttributeWriter& writer, const smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger, bool no_declaration){ std::string filecontents; writer.writeContents(resource, filecontents, logger, no_declaration); return filecontents; }, py::arg("resource"), py::arg("logger"), py::arg("no_declaration") = false)
    .def("includeDefinitions", &smtk::io::AttributeWriter::includeDefinitions, py::arg("val"))
    .def("includeInstances", &smtk::io::AttributeWriter::includeInstances, py::arg("val"))
    .def("includeViews", &smtk::io::AttributeWriter::includeViews, py::arg("val"))
    ;
  return instance;
}

#endif
