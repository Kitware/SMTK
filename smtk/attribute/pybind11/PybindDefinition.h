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

#include "smtk/attribute/Definition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::Definition > pybind11_init_smtk_attribute_Definition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Definition > instance(m, "Definition");
  instance
    .def(py::init<::smtk::attribute::Definition const &>())
    .def("deepcopy", (smtk::attribute::Definition & (smtk::attribute::Definition::*)(::smtk::attribute::Definition const &)) &smtk::attribute::Definition::operator=)
    .def("classname", &smtk::attribute::Definition::classname)
    .def("type", &smtk::attribute::Definition::type)
    .def("system", &smtk::attribute::Definition::system)
    .def("label", &smtk::attribute::Definition::label)
    .def("setLabel", &smtk::attribute::Definition::setLabel, py::arg("newLabel"))
    .def("baseDefinition", &smtk::attribute::Definition::baseDefinition)
    .def("isA", &smtk::attribute::Definition::isA, py::arg("def"))
    .def("version", &smtk::attribute::Definition::version)
    .def("setVersion", &smtk::attribute::Definition::setVersion, py::arg("myVersion"))
    .def("isAbstract", &smtk::attribute::Definition::isAbstract)
    .def("setIsAbstract", &smtk::attribute::Definition::setIsAbstract, py::arg("isAbstractValue"))
    .def("numberOfCategories", &smtk::attribute::Definition::numberOfCategories)
    .def("isMemberOf", (bool (smtk::attribute::Definition::*)(::std::string const &) const) &smtk::attribute::Definition::isMemberOf, py::arg("category"))
    .def("isMemberOf", (bool (smtk::attribute::Definition::*)(::std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > const &) const) &smtk::attribute::Definition::isMemberOf, py::arg("categories"))
    .def("categories", &smtk::attribute::Definition::categories)
    .def("advanceLevel", &smtk::attribute::Definition::advanceLevel)
    .def("setAdvanceLevel", &smtk::attribute::Definition::setAdvanceLevel, py::arg("level"))
    .def("isUnique", &smtk::attribute::Definition::isUnique)
    .def("setIsUnique", &smtk::attribute::Definition::setIsUnique, py::arg("isUniqueValue"))
    .def("isNodal", &smtk::attribute::Definition::isNodal)
    .def("setIsNodal", &smtk::attribute::Definition::setIsNodal, py::arg("isNodalValue"))
    .def("notApplicableColor", &smtk::attribute::Definition::notApplicableColor)
    .def("setNotApplicableColor", (void (smtk::attribute::Definition::*)(double, double, double, double)) &smtk::attribute::Definition::setNotApplicableColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setNotApplicableColor", (void (smtk::attribute::Definition::*)(double const *)) &smtk::attribute::Definition::setNotApplicableColor, py::arg("color"))
    .def("unsetNotApplicableColor", &smtk::attribute::Definition::unsetNotApplicableColor)
    .def("isNotApplicableColorSet", &smtk::attribute::Definition::isNotApplicableColorSet)
    .def("defaultColor", &smtk::attribute::Definition::defaultColor)
    .def("setDefaultColor", (void (smtk::attribute::Definition::*)(double, double, double, double)) &smtk::attribute::Definition::setDefaultColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setDefaultColor", (void (smtk::attribute::Definition::*)(double const *)) &smtk::attribute::Definition::setDefaultColor, py::arg("color"))
    .def("unsetDefaultColor", &smtk::attribute::Definition::unsetDefaultColor)
    .def("isDefaultColorSet", &smtk::attribute::Definition::isDefaultColorSet)
    .def("associationRule", &smtk::attribute::Definition::associationRule)
    .def("setAssociationRule", &smtk::attribute::Definition::setAssociationRule, py::arg("arg0"))
    .def("associationMask", &smtk::attribute::Definition::associationMask)
    .def("setAssociationMask", &smtk::attribute::Definition::setAssociationMask, py::arg("mask"))
    .def("associatesWithVertex", &smtk::attribute::Definition::associatesWithVertex)
    .def("associatesWithEdge", &smtk::attribute::Definition::associatesWithEdge)
    .def("associatesWithFace", &smtk::attribute::Definition::associatesWithFace)
    .def("associatesWithVolume", &smtk::attribute::Definition::associatesWithVolume)
    .def("associatesWithModel", &smtk::attribute::Definition::associatesWithModel)
    .def("associatesWithGroup", &smtk::attribute::Definition::associatesWithGroup)
    .def("canBeAssociated", (bool (smtk::attribute::Definition::*)(::smtk::model::BitFlags) const) &smtk::attribute::Definition::canBeAssociated, py::arg("maskType"))
    .def("canBeAssociated", (bool (smtk::attribute::Definition::*)(::smtk::model::EntityRef, ::std::vector<smtk::attribute::Attribute *, std::allocator<smtk::attribute::Attribute *> > *) const) &smtk::attribute::Definition::canBeAssociated, py::arg("entity"), py::arg("conflicts"))
    .def("conflicts", &smtk::attribute::Definition::conflicts, py::arg("definition"))
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
    ;
  return instance;
}

#endif
