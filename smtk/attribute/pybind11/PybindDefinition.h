//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Definition_h
#define pybind_smtk_attribute_Definition_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/Definition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Definition > pybind11_init_smtk_attribute_Definition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Definition > instance(m, "Definition");
  instance
    .def(py::init<::smtk::attribute::Definition const &>())
    .def("deepcopy", (smtk::attribute::Definition & (smtk::attribute::Definition::*)(::smtk::attribute::Definition const &)) &smtk::attribute::Definition::operator=)
    .def("type", &smtk::attribute::Definition::type)
    .def("resource", &smtk::attribute::Definition::resource)
    .def("displayedTypeName", &smtk::attribute::Definition::displayedTypeName)
    .def("label", &smtk::attribute::Definition::label)
    .def("setLabel", &smtk::attribute::Definition::setLabel, py::arg("newLabel"))
    .def("baseDefinition", &smtk::attribute::Definition::baseDefinition)
    .def("isA", &smtk::attribute::Definition::isA, py::arg("def"))
    .def("version", &smtk::attribute::Definition::version)
    .def("setVersion", &smtk::attribute::Definition::setVersion, py::arg("myVersion"))
    .def("isAbstract", &smtk::attribute::Definition::isAbstract)
    .def("setIsAbstract", &smtk::attribute::Definition::setIsAbstract, py::arg("isAbstractValue"))
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references - only non-const method of localCategories supported
    .def("categoryInheritanceMode", &smtk::attribute::Definition::categoryInheritanceMode)
    .def("setCategoryInheritanceMode", &smtk::attribute::Definition::setCategoryInheritanceMode, py::arg("categoryInheritanceModeValue"))
    .def("categories", &smtk::attribute::Definition::categories)
    .def("localCategories", (smtk::attribute::Categories::Expression& (smtk::attribute::Definition::*)()) &smtk::attribute::Definition::localCategories)
    .def("setLocalCategories", &smtk::attribute::Definition::setLocalCategories, py::arg("catExpression"))
    .def("advanceLevel", &smtk::attribute::Definition::advanceLevel, py::arg("mode") = 0)
    .def("setLocalAdvanceLevel", (void (smtk::attribute::Definition::*)(int, unsigned int)) &smtk::attribute::Definition::setLocalAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("setLocalAdvanceLevel", (void (smtk::attribute::Definition::*)(unsigned int)) &smtk::attribute::Definition::setLocalAdvanceLevel, py::arg("level"))
    .def("unsetLocalAdvanceLevel", &smtk::attribute::Definition::unsetLocalAdvanceLevel, py::arg("mode") = 0)
    .def("hasLocalAdvanceLevelInfo", &smtk::attribute::Definition::hasLocalAdvanceLevelInfo, py::arg("mode") = 0)
    .def("isUnique", &smtk::attribute::Definition::isUnique)
    .def("setIsUnique", &smtk::attribute::Definition::setIsUnique, py::arg("isUniqueValue"))
    .def("isNodal", &smtk::attribute::Definition::isNodal)
    .def("setIsNodal", &smtk::attribute::Definition::setIsNodal, py::arg("isNodalValue"))
    .def("notApplicableColor", &smtk::attribute::Definition::notApplicableColor)
    .def("setNotApplicableColor", (void (smtk::attribute::Definition::*)(double, double, double, double)) &smtk::attribute::Definition::setNotApplicableColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setNotApplicableColor", (void (smtk::attribute::Definition::*)(double const *)) &smtk::attribute::Definition::setNotApplicableColor, py::arg("color"))
    .def("unsetNotApplicableColor", &smtk::attribute::Definition::unsetNotApplicableColor)
    .def("isNotApplicableColorSet", &smtk::attribute::Definition::isNotApplicableColorSet)
    .def("isRelevant", &smtk::attribute::Definition::isRelevant, py::arg("includeCategoryCheck") = true, py::arg("includeReadAccess") = false, py::arg("readAccessLevel") = 0)
    .def("defaultColor", &smtk::attribute::Definition::defaultColor)
    .def("setDefaultColor", (void (smtk::attribute::Definition::*)(double, double, double, double)) &smtk::attribute::Definition::setDefaultColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setDefaultColor", (void (smtk::attribute::Definition::*)(double const *)) &smtk::attribute::Definition::setDefaultColor, py::arg("color"))
    .def("unsetDefaultColor", &smtk::attribute::Definition::unsetDefaultColor)
    .def("isDefaultColorSet", &smtk::attribute::Definition::isDefaultColorSet)
    .def("associationRule", &smtk::attribute::Definition::associationRule)
    .def("localAssociationRule", &smtk::attribute::Definition::localAssociationRule)
    .def("createLocalAssociationRule", &smtk::attribute::Definition::createLocalAssociationRule, py::arg("name") = "")
    .def("clearLocalAssociationRule", &smtk::attribute::Definition::clearLocalAssociationRule)
    .def("setLocalAssociationRule", (void (smtk::attribute::Definition::*)(smtk::attribute::ReferenceItemDefinitionPtr))&smtk::attribute::Definition::setLocalAssociationRule, py::arg("arg0"))
    .def("associationMask", &smtk::attribute::Definition::associationMask)
    .def("setLocalAssociationMask", &smtk::attribute::Definition::setLocalAssociationMask, py::arg("mask"))
    .def("associatesWithVertex", &smtk::attribute::Definition::associatesWithVertex)
    .def("associatesWithEdge", &smtk::attribute::Definition::associatesWithEdge)
    .def("associatesWithFace", &smtk::attribute::Definition::associatesWithFace)
    .def("associatesWithVolume", &smtk::attribute::Definition::associatesWithVolume)
    .def("associatesWithModel", &smtk::attribute::Definition::associatesWithModel)
    .def("associatesWithGroup", &smtk::attribute::Definition::associatesWithGroup)
    .def("canBeAssociated", (bool (smtk::attribute::Definition::*)(::smtk::model::BitFlags) const) &smtk::attribute::Definition::canBeAssociated, py::arg("maskType"))
    .def("canBeAssociated", (smtk::attribute::Definition::AssociationResultType (smtk::attribute::Definition::*)(::smtk::resource::ConstPersistentObjectPtr, ::smtk::attribute::AttributePtr&, ::smtk::attribute::DefinitionPtr&) const) &smtk::attribute::Definition::canBeAssociated, py::arg("entity"), py::arg("conflictingAttribute"), py::arg("requiredDefinition"))
    .def("conflicts", &smtk::attribute::Definition::conflicts, py::arg("definition"))
    .def("attributes", &smtk::attribute::Definition::attributes)
    .def("numberOfItemDefinitions", &smtk::attribute::Definition::numberOfItemDefinitions)
    .def("itemDefinition", &smtk::attribute::Definition::itemDefinition, py::arg("ith"))
    .def("addItemDefinition", (bool (smtk::attribute::Definition::*)(::smtk::attribute::ItemDefinitionPtr)) &smtk::attribute::Definition::addItemDefinition, py::arg("cdef"))
    .def("findItemPosition", &smtk::attribute::Definition::findItemPosition, py::arg("name"))
    .def("detailedDescription", &smtk::attribute::Definition::detailedDescription)
    .def("setDetailedDescription", &smtk::attribute::Definition::setDetailedDescription, py::arg("text"))
    .def("briefDescription", &smtk::attribute::Definition::briefDescription)
    .def("setBriefDescription", &smtk::attribute::Definition::setBriefDescription, py::arg("text"))
    .def("buildAttribute", &smtk::attribute::Definition::buildAttribute, py::arg("attribute"))
    .def("setRootName", &smtk::attribute::Definition::setRootName, py::arg("val"))
    .def("rootName", &smtk::attribute::Definition::rootName)
    .def("resetItemOffset", &smtk::attribute::Definition::resetItemOffset)
    .def("itemOffset", &smtk::attribute::Definition::itemOffset)
    .def("tags", &smtk::attribute::Definition::tags, py::return_value_policy::reference_internal)
    .def("tag", (smtk::attribute::Tag* (smtk::attribute::Definition::*)(const std::string&)) &smtk::attribute::Definition::tag, py::arg("name"), py::return_value_policy::reference_internal)
    .def("addTag", &smtk::attribute::Definition::addTag)
    .def("removeTag", &smtk::attribute::Definition::removeTag)
    .def("ignoreCategories", &smtk::attribute::Definition::ignoreCategories)
    .def("setIgnoreCategories", &smtk::attribute::Definition::setIgnoreCategories, py::arg("val"))
    .def("setUnits", &smtk::attribute::Definition::setUnits, py::arg("newUnits"))
    .def("units", &smtk::attribute::Definition::units)
    ;
  return instance;
}

#endif
