//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ValueItemDefinition_h
#define pybind_smtk_attribute_ValueItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ValueItem.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::ValueItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_ValueItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ValueItemDefinition, smtk::attribute::ItemDefinition > instance(m, "ValueItemDefinition");
  instance
    .def("addChildItemDefinition", &smtk::attribute::ValueItemDefinition::addChildItemDefinition, py::arg("cdef"))
    .def("addConditionalItem", &smtk::attribute::ValueItemDefinition::addConditionalItem, py::arg("enumValue"), py::arg("itemName"))
    .def("allowsExpressions", &smtk::attribute::ValueItemDefinition::allowsExpressions)
    .def("buildChildrenItems", &smtk::attribute::ValueItemDefinition::buildChildrenItems, py::arg("vitem"))
    .def("buildExpressionItem", &smtk::attribute::ValueItemDefinition::buildExpressionItem, py::arg("vitem"))
    .def("childrenItemDefinitions", &smtk::attribute::ValueItemDefinition::childrenItemDefinitions)
    .def("conditionalItems", &smtk::attribute::ValueItemDefinition::conditionalItems, py::arg("enumValue"))
    .def("defaultDiscreteIndex", &smtk::attribute::ValueItemDefinition::defaultDiscreteIndex)
    .def("discreteEnum", &smtk::attribute::ValueItemDefinition::discreteEnum, py::arg("ith"))
    .def("setEnumCategories", &smtk::attribute::ValueItemDefinition::setEnumCategories, py::arg("enumValue"), py::arg("categories"))
    .def("addEnumCategory", &smtk::attribute::ValueItemDefinition::addEnumCategory, py::arg("enumValue"), py::arg("category"))
    .def("enumCategories", &smtk::attribute::ValueItemDefinition::enumCategories, py::arg("enumValue"))
    .def("setEnumAdvanceLevel", &smtk::attribute::ValueItemDefinition::setEnumAdvanceLevel, py::arg("enum"), py::arg("level"))
    .def("unsetEnumAdvanceLevel", &smtk::attribute::ValueItemDefinition::unsetEnumAdvanceLevel, py::arg("enumValue"))
    .def("enumAdvanceLevel", &smtk::attribute::ValueItemDefinition::enumAdvanceLevel, py::arg("enumValue"))
    .def("hasEnumAdvanceLevel", &smtk::attribute::ValueItemDefinition::hasEnumAdvanceLevel, py::arg("enumValue"))
    .def("enumAdvanceLevelInfo", &smtk::attribute::ValueItemDefinition::enumAdvanceLevelInfo)
    .def("expressionDefinition", &smtk::attribute::ValueItemDefinition::expressionDefinition)
    .def("hasChildItemDefinition", (bool (smtk::attribute::ValueItemDefinition::*)(::std::string const &) const) &smtk::attribute::ValueItemDefinition::hasChildItemDefinition, py::arg("itemName"))
    .def("hasChildItemDefinition", (bool (smtk::attribute::ValueItemDefinition::*)(::std::string const &, ::std::string const &)) &smtk::attribute::ValueItemDefinition::hasChildItemDefinition, py::arg("valueName"), py::arg("itemName"))
    .def("hasDefault", &smtk::attribute::ValueItemDefinition::hasDefault)
    .def("hasRange", &smtk::attribute::ValueItemDefinition::hasRange)
    .def("hasValueLabels", &smtk::attribute::ValueItemDefinition::hasValueLabels)
    .def("isDiscrete", &smtk::attribute::ValueItemDefinition::isDiscrete)
    .def("isDiscreteIndexValid", (bool (smtk::attribute::ValueItemDefinition::*)(int index) const) &smtk::attribute::ValueItemDefinition::isDiscreteIndexValid, py::arg("index"))
    .def("isDiscreteIndexValid", (bool (smtk::attribute::ValueItemDefinition::*)(int index, const std::set<std::string>& categories) const) &smtk::attribute::ValueItemDefinition::isDiscreteIndexValid, py::arg("index"), py::arg("categories"))
    .def("isExtensible", &smtk::attribute::ValueItemDefinition::isExtensible)
    .def("isValidExpression", &smtk::attribute::ValueItemDefinition::isValidExpression, py::arg("exp"))
    .def("maxNumberOfValues", &smtk::attribute::ValueItemDefinition::maxNumberOfValues)
    .def("numberOfChildrenItemDefinitions", &smtk::attribute::ValueItemDefinition::numberOfChildrenItemDefinitions)
    .def("numberOfDiscreteValues", &smtk::attribute::ValueItemDefinition::numberOfDiscreteValues)
    .def("numberOfRequiredValues", &smtk::attribute::ValueItemDefinition::numberOfRequiredValues)
    .def("setCommonValueLabel", &smtk::attribute::ValueItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setDefaultDiscreteIndex", &smtk::attribute::ValueItemDefinition::setDefaultDiscreteIndex, py::arg("discreteIndex"))
    .def("setExpressionDefinition", &smtk::attribute::ValueItemDefinition::setExpressionDefinition, py::arg("exp"))
    .def("setIsExtensible", &smtk::attribute::ValueItemDefinition::setIsExtensible, py::arg("mode"))
    .def("setMaxNumberOfValues", &smtk::attribute::ValueItemDefinition::setMaxNumberOfValues, py::arg("esize"))
    .def("setNumberOfRequiredValues", &smtk::attribute::ValueItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setUnits", &smtk::attribute::ValueItemDefinition::setUnits, py::arg("newUnits"))
    .def("setValueLabel", &smtk::attribute::ValueItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("units", &smtk::attribute::ValueItemDefinition::units)
    .def("usingCommonLabel", &smtk::attribute::ValueItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::ValueItemDefinition::valueLabel, py::arg("element"))
    .def("hasSupportedUnits", &smtk::attribute::ValueItemDefinition::hasSupportedUnits)
    .def("supportedUnits", &smtk::attribute::ValueItemDefinition::supportedUnits)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::ValueItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
