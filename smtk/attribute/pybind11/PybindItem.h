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
#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/io/Logger.h"
#include "smtk/simulation/UserData.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Item > pybind11_init_smtk_attribute_Item(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Item > instance(m, "Item");
  instance
    .def("deepcopy", (smtk::attribute::Item & (smtk::attribute::Item::*)(::smtk::attribute::Item const &)) &smtk::attribute::Item::operator=)
    .def("_find", (smtk::attribute::ItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("_find", (smtk::attribute::ConstItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("name", &smtk::attribute::Item::name)
    .def("label", &smtk::attribute::Item::label)
    .def("type", &smtk::attribute::Item::type)
    .def("isRelevant", &smtk::attribute::Item::isRelevant, py::arg("includeCategoryChecking") = true, py::arg("includeReadAccess") = true, py::arg("readAccessLevel") = 0)
    .def("isValid", (bool (smtk::attribute::Item::*)(bool) const) &smtk::attribute::Item::isValid, py::arg("useActiveCategories") = true)
    .def("isValid", (bool (smtk::attribute::Item::*)(std::set<std::string> const &) const) &smtk::attribute::Item::isValid, py::arg("categories"))
    .def("definition", &smtk::attribute::Item::definition)
    .def("attribute", &smtk::attribute::Item::attribute)
    .def("owningItem", &smtk::attribute::Item::owningItem)
    .def("position", &smtk::attribute::Item::position)
    .def("subGroupPosition", &smtk::attribute::Item::subGroupPosition)
    .def("isOptional", &smtk::attribute::Item::isOptional)
    .def("isEnabled", &smtk::attribute::Item::isEnabled)
    .def("setIsEnabled", &smtk::attribute::Item::setIsEnabled, py::arg("isEnabledValue"))
    .def("localEnabledState", &smtk::attribute::Item::localEnabledState)
    .def("setForceRequired", &smtk::attribute::Item::setForceRequired, py::arg("forceRequiredMode"))
    .def("forceRequired", &smtk::attribute::Item::forceRequired)
    .def("setIsIgnored", &smtk::attribute::Item::setIsIgnored, py::arg("isIgnoredValue"))
    .def("isIgnored", &smtk::attribute::Item::isIgnored)
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references
    .def("categories", &smtk::attribute::Item::categories)
    .def("advanceLevel", &smtk::attribute::Item::advanceLevel, py::arg("mode") = 0)
    .def("localAdvanceLevel", &smtk::attribute::Item::localAdvanceLevel, py::arg("mode") = 0)
    .def("setLocalAdvanceLevel", &smtk::attribute::Item::setLocalAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("unsetLocalAdvanceLevel", &smtk::attribute::Item::unsetLocalAdvanceLevel, py::arg("mode") = 0)
    .def("hasLocalAdvanceLevelInfo", &smtk::attribute::Item::hasLocalAdvanceLevelInfo, py::arg("mode") = 0)
    .def("setUserData", &smtk::attribute::Item::setUserData, py::arg("key"), py::arg("value"))
    .def("userData", &smtk::attribute::Item::userData, py::arg("key"))
    .def("clearUserData", &smtk::attribute::Item::clearUserData, py::arg("key"))
    .def("clearAllUserData", &smtk::attribute::Item::clearAllUserData)
    .def("reset", &smtk::attribute::Item::reset)
    .def("detachOwningAttribute", &smtk::attribute::Item::detachOwningAttribute)
    .def("detachOwningItem", &smtk::attribute::Item::detachOwningItem)
    .def("assign", (bool (smtk::attribute::Item::*)(const ::smtk::attribute::ConstItemPtr&, const smtk::attribute::CopyAssignmentOptions&)) &smtk::attribute::Item::assign, py::arg("sourceItem"), py::arg("options") = smtk::attribute::CopyAssignmentOptions())
    .def("assign", (bool (smtk::attribute::Item::*)(const ::smtk::attribute::ConstItemPtr&, const smtk::attribute::CopyAssignmentOptions&, smtk::io::Logger&)) &smtk::attribute::Item::assign, py::arg("sourceItem"), py::arg("options"), py::arg("logger"))
    .def("assign", [](smtk::attribute::Item& item, const smtk::attribute::ConstItemPtr& sourceItem, unsigned int options)
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Item::assign(const smtk::attribute::ConstItemPtr&, unsigned int)"
        << " has been deprecated.  The replacement is Item::assign(const smtk::attribute::ConstItemPtr&, const CopyAssignmentOptions&)");
      smtk::attribute::CopyAssignmentOptions opts;
      smtk::attribute::Item::mapOldAssignmentOptions(opts, options);
      return item.assign(sourceItem, opts);
    }, py::arg("sourceItem"), py::arg("options"))

    .def("assign", (bool (smtk::attribute::Item::*)(const ::smtk::attribute::ConstItemPtr&, unsigned int)) &smtk::attribute::Item::assign, py::arg("sourceItem"), py::arg("oldOptions"))
    .def_static("type2String", &smtk::attribute::Item::type2String, py::arg("t"))
    .def_static("string2Type", &smtk::attribute::Item::string2Type, py::arg("s"))
    ;
  py::enum_<smtk::attribute::Item::Type>(instance, "Type")
    .value("AttributeRefType", smtk::attribute::Item::Type::AttributeRefType)
    .value("DoubleType", smtk::attribute::Item::Type::DoubleType)
    .value("GroupType", smtk::attribute::Item::Type::GroupType)
    .value("IntType", smtk::attribute::Item::Type::IntType)
    .value("StringType", smtk::attribute::Item::Type::StringType)
    .value("VoidType", smtk::attribute::Item::Type::VoidType)
    .value("FileType", smtk::attribute::Item::Type::FileType)
    .value("DirectoryType", smtk::attribute::Item::Type::DirectoryType)
    .value("ColorType", smtk::attribute::Item::Type::ColorType)
    .value("ModelEntityType", smtk::attribute::Item::Type::ModelEntityType)
    .value("MeshEntityType", smtk::attribute::Item::Type::MeshEntityType)
    .value("DateTimeType", smtk::attribute::Item::Type::DateTimeType)
    .value("ReferenceType", smtk::attribute::Item::Type::ReferenceType)
    .value("ResourceType", smtk::attribute::Item::Type::ResourceType)
    .value("ComponentType", smtk::attribute::Item::Type::ComponentType)
    .value("NUMBER_OF_TYPES", smtk::attribute::Item::Type::NUMBER_OF_TYPES)
    .export_values();
  py::enum_<smtk::attribute::Item::AssignmentOptions>(instance, "AssignmentOptions")
    .value("IGNORE_EXPRESSIONS", smtk::attribute::Item::AssignmentOptions::IGNORE_EXPRESSIONS)
    .value("IGNORE_MODEL_ENTITIES", smtk::attribute::Item::AssignmentOptions::IGNORE_MODEL_ENTITIES)
    .value("IGNORE_ATTRIBUTE_REF_ITEMS", smtk::attribute::Item::AssignmentOptions::IGNORE_ATTRIBUTE_REF_ITEMS)
    .value("IGNORE_RESOURCE_COMPONENTS", smtk::attribute::Item::AssignmentOptions::IGNORE_RESOURCE_COMPONENTS)
    .value("COPY_MODEL_ASSOCIATIONS", smtk::attribute::Item::AssignmentOptions::COPY_MODEL_ASSOCIATIONS)
    .export_values();
  return instance;
}

#endif
