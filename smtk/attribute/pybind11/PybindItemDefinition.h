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
#include <pybind11/stl.h>

#include "smtk/attribute/ItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_ItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ItemDefinition > instance(m, "ItemDefinition");
  instance
    .def("advanceLevel", &smtk::attribute::ItemDefinition::advanceLevel, py::arg("mode") = 0)
    .def("briefDescription", &smtk::attribute::ItemDefinition::briefDescription)
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::ItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::ItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references - oly non-const method of localCategories supported
    .def("categories", &smtk::attribute::ItemDefinition::categories)
    .def("localCategories", (smtk::attribute::Categories::Expression& (smtk::attribute::ItemDefinition::*)()) &smtk::attribute::ItemDefinition::localCategories, py::return_value_policy::reference)
    .def("setLocalCategories", &smtk::attribute::ItemDefinition::setLocalCategories, py::arg("catExpression"))
    .def("createCopy", &smtk::attribute::ItemDefinition::createCopy, py::arg("info"))
    .def("detailedDescription", &smtk::attribute::ItemDefinition::detailedDescription)
    .def("isEnabledByDefault", &smtk::attribute::ItemDefinition::isEnabledByDefault)
    .def("categoryInheritanceMode", &smtk::attribute::ItemDefinition::categoryInheritanceMode)
    .def("isOptional", &smtk::attribute::ItemDefinition::isOptional)
    .def("label", &smtk::attribute::ItemDefinition::label)
    .def("name", &smtk::attribute::ItemDefinition::name)
    .def("advanceLevel", &smtk::attribute::ItemDefinition::advanceLevel, py::arg("mode") = 0)
    .def("setLocalAdvanceLevel", (void (smtk::attribute::ItemDefinition::*)(int, unsigned int)) &smtk::attribute::ItemDefinition::setLocalAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("setLocalAdvanceLevel", (void (smtk::attribute::ItemDefinition::*)(unsigned int)) &smtk::attribute::ItemDefinition::setLocalAdvanceLevel, py::arg("level"))
    .def("unsetLocalAdvanceLevel", &smtk::attribute::ItemDefinition::unsetLocalAdvanceLevel, py::arg("mode") = 0)
    .def("hasLocalAdvanceLevelInfo", &smtk::attribute::ItemDefinition::hasLocalAdvanceLevelInfo, py::arg("mode") = 0)
    .def("setBriefDescription", &smtk::attribute::ItemDefinition::setBriefDescription, py::arg("text"))
    .def("setDetailedDescription", &smtk::attribute::ItemDefinition::setDetailedDescription, py::arg("text"))
    .def("setIsEnabledByDefault", &smtk::attribute::ItemDefinition::setIsEnabledByDefault, py::arg("isEnabledByDefaultValue"))
    .def("setCategoryInheritanceMode", &smtk::attribute::ItemDefinition::setCategoryInheritanceMode, py::arg("categoryInheritanceModeValue"))
    .def("setIsOptional", &smtk::attribute::ItemDefinition::setIsOptional, py::arg("isOptionalValue"))
    .def("setLabel", &smtk::attribute::ItemDefinition::setLabel, py::arg("newLabel"))
    .def("setVersion", &smtk::attribute::ItemDefinition::setVersion, py::arg("myVersion"))
    .def("type", &smtk::attribute::ItemDefinition::type)
    .def("version", &smtk::attribute::ItemDefinition::version)
    .def("tags", &smtk::attribute::ItemDefinition::tags, py::return_value_policy::reference_internal)
    .def("tag", (smtk::attribute::Tag* (smtk::attribute::ItemDefinition::*)(const std::string&)) &smtk::attribute::ItemDefinition::tag, py::arg("name"), py::return_value_policy::reference_internal)
    .def("addTag", &smtk::attribute::ItemDefinition::addTag)
    .def("removeTag", &smtk::attribute::ItemDefinition::removeTag)
    ;
  PySharedPtrClass< smtk::attribute::ItemDefinition::CopyInfo >(instance, "CopyInfo")
    .def(py::init<::smtk::attribute::ResourcePtr>())
    .def(py::init<::smtk::attribute::ItemDefinition::CopyInfo const &>())
    // .def_readwrite("ToResource", &smtk::attribute::ItemDefinition::CopyInfo::ToResource)
    .def_readwrite("UnresolvedExpItems", &smtk::attribute::ItemDefinition::CopyInfo::UnresolvedExpItems)
    ;
  return instance;
}

#endif
