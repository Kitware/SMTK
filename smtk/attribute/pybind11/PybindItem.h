//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Item_h
#define pybind_smtk_attribute_Item_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Item.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/simulation/UserData.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::Item > pybind11_init_smtk_attribute_Item(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Item > instance(m, "Item");
  instance
    .def("deepcopy", (smtk::attribute::Item & (smtk::attribute::Item::*)(::smtk::attribute::Item const &)) &smtk::attribute::Item::operator=)
    .def("classname", &smtk::attribute::Item::classname)
    .def("name", &smtk::attribute::Item::name)
    .def("label", &smtk::attribute::Item::label)
    .def("type", &smtk::attribute::Item::type)
    .def("isValid", &smtk::attribute::Item::isValid)
    .def("definition", &smtk::attribute::Item::definition)
    .def("attribute", &smtk::attribute::Item::attribute)
    .def("owningItem", &smtk::attribute::Item::owningItem)
    .def("position", &smtk::attribute::Item::position)
    .def("subGroupPosition", &smtk::attribute::Item::subGroupPosition)
    .def("isOptional", &smtk::attribute::Item::isOptional)
    .def("isEnabled", &smtk::attribute::Item::isEnabled)
    .def("setIsEnabled", &smtk::attribute::Item::setIsEnabled, py::arg("isEnabledValue"))
    .def("isMemberOf", (bool (smtk::attribute::Item::*)(::std::string const &) const) &smtk::attribute::Item::isMemberOf, py::arg("category"))
    .def("isMemberOf", (bool (smtk::attribute::Item::*)(::std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > const &) const) &smtk::attribute::Item::isMemberOf, py::arg("categories"))
    .def("advanceLevel", &smtk::attribute::Item::advanceLevel, py::arg("mode") = 0)
    .def("setAdvanceLevel", &smtk::attribute::Item::setAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("unsetAdvanceLevel", &smtk::attribute::Item::unsetAdvanceLevel, py::arg("mode") = 0)
    .def("usingDefinitionAdvanceLevel", &smtk::attribute::Item::usingDefinitionAdvanceLevel, py::arg("mode") = 0)
    .def("setUserData", &smtk::attribute::Item::setUserData, py::arg("key"), py::arg("value"))
    .def("userData", &smtk::attribute::Item::userData, py::arg("key"))
    .def("clearUserData", &smtk::attribute::Item::clearUserData, py::arg("key"))
    .def("clearAllUserData", &smtk::attribute::Item::clearAllUserData)
    .def("reset", &smtk::attribute::Item::reset)
    .def("detachOwningAttribute", &smtk::attribute::Item::detachOwningAttribute)
    .def("detachOwningItem", &smtk::attribute::Item::detachOwningItem)
    .def("assign", &smtk::attribute::Item::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def_static("type2String", &smtk::attribute::Item::type2String, py::arg("t"))
    .def_static("string2Type", &smtk::attribute::Item::string2Type, py::arg("s"))
    ;
  py::enum_<smtk::attribute::Item::Type>(instance, "Type")
    .value("ATTRIBUTE_REF", smtk::attribute::Item::Type::ATTRIBUTE_REF)
    .value("DOUBLE", smtk::attribute::Item::Type::DOUBLE)
    .value("GROUP", smtk::attribute::Item::Type::GROUP)
    .value("INT", smtk::attribute::Item::Type::INT)
    .value("STRING", smtk::attribute::Item::Type::STRING)
    .value("VOID", smtk::attribute::Item::Type::VOID)
    .value("FILE", smtk::attribute::Item::Type::FILE)
    .value("DIRECTORY", smtk::attribute::Item::Type::DIRECTORY)
    .value("COLOR", smtk::attribute::Item::Type::COLOR)
    .value("MODEL_ENTITY", smtk::attribute::Item::Type::MODEL_ENTITY)
    .value("MESH_SELECTION", smtk::attribute::Item::Type::MESH_SELECTION)
    .value("MESH_ENTITY", smtk::attribute::Item::Type::MESH_ENTITY)
    .value("NUMBER_OF_TYPES", smtk::attribute::Item::Type::NUMBER_OF_TYPES)
    .export_values();
  py::enum_<smtk::attribute::Item::AssignmentOptions>(instance, "AssignmentOptions")
    .value("IGNORE_EXPRESSIONS", smtk::attribute::Item::AssignmentOptions::IGNORE_EXPRESSIONS)
    .value("IGNORE_MODEL_ENTITIES", smtk::attribute::Item::AssignmentOptions::IGNORE_MODEL_ENTITIES)
    .value("IGNORE_ATTRIBUTE_REF_ITEMS", smtk::attribute::Item::AssignmentOptions::IGNORE_ATTRIBUTE_REF_ITEMS)
    .value("COPY_MODEL_ASSOCIATIONS", smtk::attribute::Item::AssignmentOptions::COPY_MODEL_ASSOCIATIONS)
    .export_values();
  return instance;
}

#endif
