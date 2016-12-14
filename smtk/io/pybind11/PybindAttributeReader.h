//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_AttributeReader_h
#define pybind_smtk_io_AttributeReader_h

#include <pybind11/pybind11.h>

#include "smtk/io/AttributeReader.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::AttributeReader > pybind11_init_smtk_io_AttributeReader(py::module &m)
{
  PySharedPtrClass< smtk::io::AttributeReader > instance(m, "AttributeReader");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::AttributeReader const &>())
    .def("deepcopy", (smtk::io::AttributeReader & (smtk::io::AttributeReader::*)(::smtk::io::AttributeReader const &)) &smtk::io::AttributeReader::operator=)
    .def("read", (bool (smtk::io::AttributeReader::*)(::smtk::attribute::System &, ::std::string const &, bool, ::smtk::io::Logger &)) &smtk::io::AttributeReader::read, py::arg("system"), py::arg("filename"), py::arg("includePath"), py::arg("logger"))
    .def("read", (bool (smtk::io::AttributeReader::*)(::smtk::attribute::System &, ::std::string const &, ::smtk::io::Logger &)) &smtk::io::AttributeReader::read, py::arg("system"), py::arg("filename"), py::arg("logger"))
    .def("readContents", (bool (smtk::io::AttributeReader::*)(::smtk::attribute::System &, ::std::string const &, ::smtk::io::Logger &)) &smtk::io::AttributeReader::readContents, py::arg("system"), py::arg("filecontents"), py::arg("logger"))
    .def("readContents", (bool (smtk::io::AttributeReader::*)(::smtk::attribute::System &, char const *, ::size_t, ::smtk::io::Logger &)) &smtk::io::AttributeReader::readContents, py::arg("system"), py::arg("contents"), py::arg("length"), py::arg("logger"))
    .def("readContents", (bool (smtk::io::AttributeReader::*)(::smtk::attribute::System &, ::pugi::xml_node &, ::smtk::io::Logger &)) &smtk::io::AttributeReader::readContents, py::arg("system"), py::arg("rootNode"), py::arg("logger"))
    .def("setSearchPaths", &smtk::io::AttributeReader::setSearchPaths, py::arg("paths"))
    .def("setReportDuplicateDefinitionsAsErrors", &smtk::io::AttributeReader::setReportDuplicateDefinitionsAsErrors, py::arg("mode"))
    ;
  return instance;
}

#endif
