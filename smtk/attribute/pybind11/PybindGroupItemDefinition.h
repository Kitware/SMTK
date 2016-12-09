//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_GroupItemDefinition_h
#define pybind_smtk_attribute_GroupItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/GroupItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::GroupItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_GroupItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::GroupItemDefinition, smtk::attribute::ItemDefinition > instance(m, "GroupItemDefinition");
  instance
    .def(py::init<::smtk::attribute::GroupItemDefinition const &>())
    .def_static("New", &smtk::attribute::GroupItemDefinition::New, py::arg("myName"))
    .def("addCategory", &smtk::attribute::GroupItemDefinition::addCategory, py::arg("category"))
    .def("addItemDefinition", (bool (smtk::attribute::GroupItemDefinition::*)(::smtk::attribute::ItemDefinitionPtr)) &smtk::attribute::GroupItemDefinition::addItemDefinition, py::arg("cdef"))
    .def("buildGroup", &smtk::attribute::GroupItemDefinition::buildGroup, py::arg("group"), py::arg("subGroupPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::GroupItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::GroupItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::GroupItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::GroupItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::GroupItemDefinition::classname)
    .def("createCopy", &smtk::attribute::GroupItemDefinition::createCopy, py::arg("info"))
    .def("findItemPosition", &smtk::attribute::GroupItemDefinition::findItemPosition, py::arg("name"))
    .def("hasSubGroupLabels", &smtk::attribute::GroupItemDefinition::hasSubGroupLabels)
    .def("isExtensible", &smtk::attribute::GroupItemDefinition::isExtensible)
    .def("itemDefinition", &smtk::attribute::GroupItemDefinition::itemDefinition, py::arg("ith"))
    .def("maxNumberOfGroups", &smtk::attribute::GroupItemDefinition::maxNumberOfGroups)
    .def("numberOfItemDefinitions", &smtk::attribute::GroupItemDefinition::numberOfItemDefinitions)
    .def("numberOfRequiredGroups", &smtk::attribute::GroupItemDefinition::numberOfRequiredGroups)
    .def("removeCategory", &smtk::attribute::GroupItemDefinition::removeCategory, py::arg("category"))
    .def("setCommonSubGroupLabel", &smtk::attribute::GroupItemDefinition::setCommonSubGroupLabel, py::arg("elabel"))
    .def("setIsExtensible", &smtk::attribute::GroupItemDefinition::setIsExtensible, py::arg("mode"))
    .def("setMaxNumberOfGroups", &smtk::attribute::GroupItemDefinition::setMaxNumberOfGroups, py::arg("esize"))
    .def("setNumberOfRequiredGroups", &smtk::attribute::GroupItemDefinition::setNumberOfRequiredGroups, py::arg("gsize"))
    .def("setSubGroupLabel", &smtk::attribute::GroupItemDefinition::setSubGroupLabel, py::arg("element"), py::arg("elabel"))
    .def("subGroupLabel", &smtk::attribute::GroupItemDefinition::subGroupLabel, py::arg("element"))
    .def("type", &smtk::attribute::GroupItemDefinition::type)
    .def("usingCommonSubGroupLabel", &smtk::attribute::GroupItemDefinition::usingCommonSubGroupLabel)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::GroupItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
