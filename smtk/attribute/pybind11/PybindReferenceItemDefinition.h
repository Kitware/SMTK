//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ReferenceItemDefinition_h
#define pybind_smtk_attribute_ReferenceItemDefinition_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::attribute::ReferenceItemDefinition, smtk::attribute::ItemDefinition>
pybind11_init_smtk_attribute_ReferenceItemDefinition(py::module& m)
{
  PySharedPtrClass<smtk::attribute::ReferenceItemDefinition, smtk::attribute::ItemDefinition>
    instance(m, "ReferenceItemDefinition");
  instance.def(py::init< ::smtk::attribute::ReferenceItemDefinition const&>())
    .def_static("New", &smtk::attribute::ReferenceItemDefinition::New, py::arg("name"))
    .def("acceptableEntries", &smtk::attribute::ReferenceItemDefinition::acceptableEntries)
    .def("addChildItemDefinition", &smtk::attribute::ReferenceItemDefinition::addChildItemDefinition, py::arg("cdef"))
    .def("addConditional", &smtk::attribute::ReferenceItemDefinition::addConditional, py::arg("resourceQuery"), py::arg("conditionalQuery"), py::arg("itemNames"))
    .def("buildChildrenItems", &smtk::attribute::ReferenceItemDefinition::buildChildrenItems, py::arg("ritem"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ReferenceItemDefinition::*)(
                        ::smtk::attribute::Attribute*, int) const) &
        smtk::attribute::ReferenceItemDefinition::buildItem,
      py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ReferenceItemDefinition::*)(
                        ::smtk::attribute::Item*, int, int) const) &
        smtk::attribute::ReferenceItemDefinition::buildItem,
      py::arg("owner"), py::arg("itemPos"), py::arg("subGroupPosition"))
    .def("childrenItemDefinitions", &smtk::attribute::ReferenceItemDefinition::childrenItemDefinitions)
    .def(
      "clearAcceptableEntries", &smtk::attribute::ReferenceItemDefinition::clearAcceptableEntries)
    .def("clearRejectedEntries", &smtk::attribute::ReferenceItemDefinition::clearRejectedEntries)
    .def("componentQueries", &smtk::attribute::ReferenceItemDefinition::componentQueries)
    .def("conditionalInformation", &smtk::attribute::ReferenceItemDefinition::conditionalInformation)
    .def("conditionalItems", &smtk::attribute::ReferenceItemDefinition::conditionalItems, py::arg("index"))
    .def("createCopy", &smtk::attribute::ReferenceItemDefinition::createCopy, py::arg("info"))
    .def("hasChildItemDefinition", (bool (smtk::attribute::ReferenceItemDefinition::*)(::std::string const &) const) &smtk::attribute::ReferenceItemDefinition::hasChildItemDefinition, py::arg("itemName"))
    .def("hasChildItemDefinition", (bool (smtk::attribute::ReferenceItemDefinition::*)(::std::size_t, ::std::string const &)) &smtk::attribute::ReferenceItemDefinition::hasChildItemDefinition, py::arg("conditionalIndex"), py::arg("itemName"))
    .def("hasValueLabels", &smtk::attribute::ReferenceItemDefinition::hasValueLabels)
    .def("holdReference", (bool (smtk::attribute::ReferenceItemDefinition::*)() const) &
        smtk::attribute::ReferenceItemDefinition::holdReference)
    .def("setHoldReference", (void (smtk::attribute::ReferenceItemDefinition::*)(bool)) &
        smtk::attribute::ReferenceItemDefinition::setHoldReference)
    .def("isExtensible", &smtk::attribute::ReferenceItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::ReferenceItemDefinition::isValueValid, py::arg("entity"))
    .def("lockType", &smtk::attribute::ReferenceItemDefinition::lockType)
    .def("maxNumberOfValues", &smtk::attribute::ReferenceItemDefinition::maxNumberOfValues)
    .def("numberOfChildrenItemDefinitions", &smtk::attribute::ReferenceItemDefinition::numberOfChildrenItemDefinitions)
    .def("numberOfConditionals", &smtk::attribute::ReferenceItemDefinition::numberOfConditionals)
    .def(
      "numberOfRequiredValues", &smtk::attribute::ReferenceItemDefinition::numberOfRequiredValues)
    .def("rejectedEntries", &smtk::attribute::ReferenceItemDefinition::rejectedEntries)
    .def("resourceQueries", &smtk::attribute::ReferenceItemDefinition::resourceQueries)
    .def("role", &smtk::attribute::ReferenceItemDefinition::role)
    .def("setAcceptsEntries", &smtk::attribute::ReferenceItemDefinition::setAcceptsEntries,
      py::arg("typeName"), py::arg("queryString"), py::arg("accept"))
    .def("setCommonValueLabel", &smtk::attribute::ReferenceItemDefinition::setCommonValueLabel,
      py::arg("elabel"))
    .def("enforcesCategories", &smtk::attribute::ReferenceItemDefinition::enforcesCategories)
    .def("setEnforcesCategories", &smtk::attribute::ReferenceItemDefinition::setEnforcesCategories,
      py::arg("enforcesCategoriesMode"))
    .def("setIsExtensible", &smtk::attribute::ReferenceItemDefinition::setIsExtensible,
      py::arg("extensible"))
    .def("setLockType", &smtk::attribute::ReferenceItemDefinition::setLockType, py::arg("val"))
    .def("setMaxNumberOfValues", &smtk::attribute::ReferenceItemDefinition::setMaxNumberOfValues,
      py::arg("maxNum"))
    .def("setNumberOfRequiredValues",
      &smtk::attribute::ReferenceItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setRejectsEntries", &smtk::attribute::ReferenceItemDefinition::setAcceptsEntries,
      py::arg("typeName"), py::arg("queryString"), py::arg("add"))
    .def("setValueLabel", &smtk::attribute::ReferenceItemDefinition::setValueLabel,
      py::arg("element"), py::arg("elabel"))
    .def("testConditionals", &smtk::attribute::ReferenceItemDefinition::testConditionals, py::arg("object"))
    .def("type", &smtk::attribute::ReferenceItemDefinition::type)
    .def("typeName", &smtk::attribute::ReferenceItemDefinition::typeName)
    .def("usingCommonLabel", &smtk::attribute::ReferenceItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::ReferenceItemDefinition::valueLabel, py::arg("element"));
  return instance;
}

#endif
