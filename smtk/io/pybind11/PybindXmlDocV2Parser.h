//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_XmlDocV2Parser_h
#define pybind_smtk_io_XmlDocV2Parser_h

#include <pybind11/pybind11.h>

#include "smtk/io/XmlDocV2Parser.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::XmlDocV2Parser > pybind11_init_smtk_io_XmlDocV2Parser(py::module &m, PySharedPtrClass< smtk::io::XmlDocV1Parser >& parent)
{
  PySharedPtrClass< smtk::io::XmlDocV2Parser > instance(m, "XmlDocV2Parser", parent);
  instance
    .def(py::init<::smtk::io::XmlDocV2Parser const &>())
    .def(py::init<::smtk::attribute::SystemPtr>())
    .def_static("canParse", (bool (*)(::pugi::xml_node &)) &smtk::io::XmlDocV2Parser::canParse, py::arg("node"))
    .def_static("canParse", (bool (*)(::pugi::xml_document &)) &smtk::io::XmlDocV2Parser::canParse, py::arg("doc"))
    .def_static("getRootNode", &smtk::io::XmlDocV2Parser::getRootNode, py::arg("doc"))
    .def("process", (void (smtk::io::XmlDocV2Parser::*)(::pugi::xml_document &)) &smtk::io::XmlDocV2Parser::process, py::arg("doc"))
    .def("process", (void (smtk::io::XmlDocV2Parser::*)(::pugi::xml_node &)) &smtk::io::XmlDocV2Parser::process, py::arg("rootNode"))
    ;
  return instance;
}

#endif
