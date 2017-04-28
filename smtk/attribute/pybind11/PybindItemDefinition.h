//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ItemDefinition_h
#define pybind_smtk_attribute_ItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_ItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ItemDefinition > instance(m, "ItemDefinition");
  instance
    .def("addCategory", &smtk::attribute::ItemDefinition::addCategory, py::arg("category"))
    .def("advanceLevel", &smtk::attribute::ItemDefinition::advanceLevel, py::arg("mode") = 0)
    .def("briefDescription", &smtk::attribute::ItemDefinition::briefDescription)
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::ItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::ItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("categories", &smtk::attribute::ItemDefinition::categories)
    .def("classname", &smtk::attribute::ItemDefinition::classname)
    .def("createCopy", &smtk::attribute::ItemDefinition::createCopy, py::arg("info"))
    .def("detailedDescription", &smtk::attribute::ItemDefinition::detailedDescription)
    .def("isEnabledByDefault", &smtk::attribute::ItemDefinition::isEnabledByDefault)
    .def("isMemberOf", (bool (smtk::attribute::ItemDefinition::*)(::std::string const &) const) &smtk::attribute::ItemDefinition::isMemberOf, py::arg("category"))
    .def("isMemberOf", (bool (smtk::attribute::ItemDefinition::*)(::std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > const &) const) &smtk::attribute::ItemDefinition::isMemberOf, py::arg("categories"))
    .def("isOptional", &smtk::attribute::ItemDefinition::isOptional)
    .def("label", &smtk::attribute::ItemDefinition::label)
    .def("name", &smtk::attribute::ItemDefinition::name)
    .def("numberOfCategories", &smtk::attribute::ItemDefinition::numberOfCategories)
    .def("removeCategory", &smtk::attribute::ItemDefinition::removeCategory, py::arg("category"))
    .def("setAdvanceLevel", (void (smtk::attribute::ItemDefinition::*)(int, int)) &smtk::attribute::ItemDefinition::setAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("setAdvanceLevel", (void (smtk::attribute::ItemDefinition::*)(int)) &smtk::attribute::ItemDefinition::setAdvanceLevel, py::arg("level"))
    .def("setBriefDescription", &smtk::attribute::ItemDefinition::setBriefDescription, py::arg("text"))
    .def("setDetailedDescription", &smtk::attribute::ItemDefinition::setDetailedDescription, py::arg("text"))
    .def("setIsEnabledByDefault", &smtk::attribute::ItemDefinition::setIsEnabledByDefault, py::arg("isEnabledByDefaultValue"))
    .def("setIsOptional", &smtk::attribute::ItemDefinition::setIsOptional, py::arg("isOptionalValue"))
    .def("setLabel", &smtk::attribute::ItemDefinition::setLabel, py::arg("newLabel"))
    .def("setVersion", &smtk::attribute::ItemDefinition::setVersion, py::arg("myVersion"))
    .def("type", &smtk::attribute::ItemDefinition::type)
    .def("version", &smtk::attribute::ItemDefinition::version)
    ;
  PySharedPtrClass< smtk::attribute::ItemDefinition::CopyInfo >(instance, "CopyInfo")
    .def(py::init<::smtk::attribute::SystemPtr>())
    .def(py::init<::smtk::attribute::ItemDefinition::CopyInfo const &>())
    // .def_readwrite("ToSystem", &smtk::attribute::ItemDefinition::CopyInfo::ToSystem)
    .def_readwrite("UnresolvedExpItems", &smtk::attribute::ItemDefinition::CopyInfo::UnresolvedExpItems)
    .def_readwrite("UnresolvedRefItems", &smtk::attribute::ItemDefinition::CopyInfo::UnresolvedRefItems)
    ;
  return instance;
}

#endif
