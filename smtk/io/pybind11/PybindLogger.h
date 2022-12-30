//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_Logger_h
#define pybind_smtk_io_Logger_h

#include <pybind11/pybind11.h>

#include "smtk/io/Logger.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::io::Logger > pybind11_init_smtk_io_Logger(py::module &m)
{
  PySharedPtrClass< smtk::io::Logger > instance(m, "Logger");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::Logger const &>())
    .def("deepcopy", (smtk::io::Logger & (smtk::io::Logger::*)(::smtk::io::Logger const &)) &smtk::io::Logger::operator=)
    .def_static("instance", &smtk::io::Logger::instance, pybind11::return_value_policy::reference)
    .def("numberOfRecords", &smtk::io::Logger::numberOfRecords)
    .def("hasErrors", &smtk::io::Logger::hasErrors)
    .def("clearErrors", &smtk::io::Logger::clearErrors)
    .def("addRecord", &smtk::io::Logger::addRecord, py::arg("s"), py::arg("m"), py::arg("fname") = "", py::arg("line") = 0)
    .def("record", &smtk::io::Logger::record, py::arg("i"))
    .def("toString", (std::string (smtk::io::Logger::*)(::size_t, bool) const) &smtk::io::Logger::toString, py::arg("i"), py::arg("includeSourceLoc") = false)
    .def("toString", (std::string (smtk::io::Logger::*)(::size_t, ::size_t, bool) const) &smtk::io::Logger::toString, py::arg("i"), py::arg("j"), py::arg("includeSourceLoc") = false)
    .def("toHTML", &smtk::io::Logger::toHTML, py::arg("i"), py::arg("j"), py::arg("includeSourceLoc"))
    .def("convertToString", &smtk::io::Logger::convertToString, py::arg("includeSourceLoc") = false)
    .def("convertToHTML", &smtk::io::Logger::convertToHTML, py::arg("includeSourceLoc") = false)
    .def("reset", &smtk::io::Logger::reset)
    .def("append", &smtk::io::Logger::append, py::arg("l"))
    .def_static("severityAsString", &smtk::io::Logger::severityAsString, py::arg("s"))
    .def("setFlushToStream", &smtk::io::Logger::setFlushToStream, py::arg("output"), py::arg("ownFile"), py::arg("includePast"))
    .def("setFlushToFile", &smtk::io::Logger::setFlushToFile, py::arg("filename"), py::arg("includePast"))
    .def("setFlushToStdout", &smtk::io::Logger::setFlushToStdout, py::arg("includePast"))
    .def("setFlushToStderr", &smtk::io::Logger::setFlushToStderr, py::arg("includePast"))
    ;
  PySharedPtrClass< smtk::io::Logger::Record >(instance, "Record")
    .def(py::init<::smtk::io::Logger::Severity, ::std::string const &, ::std::string const &, unsigned int>())
    .def(py::init<>())
    .def(py::init<::smtk::io::Logger::Record const &>())
    .def("deepcopy", (smtk::io::Logger::Record & (smtk::io::Logger::Record::*)(::smtk::io::Logger::Record const &)) &smtk::io::Logger::Record::operator=)
    .def_readwrite("severity", &smtk::io::Logger::Record::severity)
    .def_readwrite("message", &smtk::io::Logger::Record::message)
    .def_readwrite("fileName", &smtk::io::Logger::Record::fileName)
    .def_readwrite("lineNumber", &smtk::io::Logger::Record::lineNumber)
    ;
  py::enum_<smtk::io::Logger::Severity>(instance, "Severity")
    .value("Debug", smtk::io::Logger::Severity::Debug)
    .value("Info", smtk::io::Logger::Severity::Info)
    .value("Warning", smtk::io::Logger::Severity::Warning)
    .value("Error", smtk::io::Logger::Severity::Error)
    .value("Fatal", smtk::io::Logger::Severity::Fatal)
    .export_values();
  return instance;
}

#endif
