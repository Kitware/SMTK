//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_XmlDocV1Parser_h
#define pybind_smtk_io_XmlDocV1Parser_h

#include <pybind11/pybind11.h>

#include "smtk/io/XmlDocV1Parser.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::AttRefInfo > pybind11_init_smtk_io_AttRefInfo(py::module &m)
{
  PySharedPtrClass< smtk::io::AttRefInfo > instance(m, "AttRefInfo");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::AttRefInfo const &>())
    .def("deepcopy", (smtk::io::AttRefInfo & (smtk::io::AttRefInfo::*)(::smtk::io::AttRefInfo const &)) &smtk::io::AttRefInfo::operator=)
    .def_readwrite("attName", &smtk::io::AttRefInfo::attName)
    .def_readwrite("item", &smtk::io::AttRefInfo::item)
    .def_readwrite("pos", &smtk::io::AttRefInfo::pos)
    ;
  return instance;
}

PySharedPtrClass< smtk::io::ItemExpressionInfo > pybind11_init_smtk_io_ItemExpressionInfo(py::module &m)
{
  PySharedPtrClass< smtk::io::ItemExpressionInfo > instance(m, "ItemExpressionInfo");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ItemExpressionInfo const &>())
    .def("deepcopy", (smtk::io::ItemExpressionInfo & (smtk::io::ItemExpressionInfo::*)(::smtk::io::ItemExpressionInfo const &)) &smtk::io::ItemExpressionInfo::operator=)
    .def_readwrite("expName", &smtk::io::ItemExpressionInfo::expName)
    .def_readwrite("item", &smtk::io::ItemExpressionInfo::item)
    .def_readwrite("pos", &smtk::io::ItemExpressionInfo::pos)
    ;
  return instance;
}

PySharedPtrClass< smtk::io::XmlDocV1Parser > pybind11_init_smtk_io_XmlDocV1Parser(py::module &m)
{
  PySharedPtrClass< smtk::io::XmlDocV1Parser > instance(m, "XmlDocV1Parser");
  instance
    .def(py::init<::smtk::io::XmlDocV1Parser const &>())
    .def(py::init<::smtk::attribute::SystemPtr>())
    .def_static("canParse", (bool (*)(::pugi::xml_document &)) &smtk::io::XmlDocV1Parser::canParse, py::arg("doc"))
    .def_static("canParse", (bool (*)(::pugi::xml_node &)) &smtk::io::XmlDocV1Parser::canParse, py::arg("node"))
    .def_static("getRootNode", &smtk::io::XmlDocV1Parser::getRootNode, py::arg("doc"))
    .def("messageLog", &smtk::io::XmlDocV1Parser::messageLog)
    .def("process", (void (smtk::io::XmlDocV1Parser::*)(::pugi::xml_document &)) &smtk::io::XmlDocV1Parser::process, py::arg("doc"))
    .def("process", (void (smtk::io::XmlDocV1Parser::*)(::pugi::xml_node &)) &smtk::io::XmlDocV1Parser::process, py::arg("rootNode"))
    .def("setReportDuplicateDefinitionsAsErrors", &smtk::io::XmlDocV1Parser::setReportDuplicateDefinitionsAsErrors, py::arg("mode"))
    ;
  return instance;
}

#endif
