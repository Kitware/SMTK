//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ComponentItemDefinition_h
#define pybind_smtk_attribute_ComponentItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/resource/Component.h"

namespace py = pybind11;

py::class_< smtk::attribute::ComponentItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_ComponentItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ComponentItemDefinition, smtk::attribute::ItemDefinition > instance(m, "ComponentItemDefinition");
  instance
    .def(py::init<::smtk::attribute::ComponentItemDefinition const &>())
    .def_static("New", &smtk::attribute::ComponentItemDefinition::New, py::arg("sname"))
    .def("acceptsResourceComponents", (bool (smtk::attribute::ComponentItemDefinition::*)(::std::string const &) const) &smtk::attribute::ComponentItemDefinition::acceptsResourceComponents, py::arg("uniqueName"))
    .def("acceptsResourceComponents", (bool (smtk::attribute::ComponentItemDefinition::*)(::std::type_index) const) &smtk::attribute::ComponentItemDefinition::acceptsResourceComponents, py::arg("resourceIndex"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ComponentItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::ComponentItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ComponentItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::ComponentItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::ComponentItemDefinition::classname)
    .def("createCopy", &smtk::attribute::ComponentItemDefinition::createCopy, py::arg("info"))
    .def("hasValueLabels", &smtk::attribute::ComponentItemDefinition::hasValueLabels)
    .def("isExtensible", &smtk::attribute::ComponentItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::ComponentItemDefinition::isValueValid, py::arg("entity"))
    .def("maxNumberOfValues", &smtk::attribute::ComponentItemDefinition::maxNumberOfValues)
    .def("membershipMask", &smtk::attribute::ComponentItemDefinition::membershipMask)
    .def("numberOfRequiredValues", &smtk::attribute::ComponentItemDefinition::numberOfRequiredValues)
    .def("setAcceptsResourceComponents", (bool (smtk::attribute::ComponentItemDefinition::*)(::std::string const &, bool)) &smtk::attribute::ComponentItemDefinition::setAcceptsResourceComponents, py::arg("uniqueName"), py::arg("accept"))
    .def("setAcceptsResourceComponents", (bool (smtk::attribute::ComponentItemDefinition::*)(::std::type_index, bool)) &smtk::attribute::ComponentItemDefinition::setAcceptsResourceComponents, py::arg("resourceIndex"), py::arg("accept"))
    .def("setCommonValueLabel", &smtk::attribute::ComponentItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setIsExtensible", &smtk::attribute::ComponentItemDefinition::setIsExtensible, py::arg("extensible"))
    .def("setMaxNumberOfValues", &smtk::attribute::ComponentItemDefinition::setMaxNumberOfValues, py::arg("maxNum"))
    .def("setMembershipMask", &smtk::attribute::ComponentItemDefinition::setMembershipMask, py::arg("entMask"))
    .def("setNumberOfRequiredValues", &smtk::attribute::ComponentItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setValueLabel", &smtk::attribute::ComponentItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("type", &smtk::attribute::ComponentItemDefinition::type)
    .def("usingCommonLabel", &smtk::attribute::ComponentItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::ComponentItemDefinition::valueLabel, py::arg("element"))

    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::ComponentItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
