//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_attribute_DateTimeItemDefinition_h
#define pybind___smtk_attribute_DateTimeItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DateTimeItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::DateTimeItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<smtk::common::DateTimeZonePair> > pybind11_init_smtk_attribute_DateTimeItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DateTimeItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<smtk::common::DateTimeZonePair> > instance(m, "DateTimeItemDefinition");
  instance
    .def(py::init<::smtk::attribute::DateTimeItemDefinition const &>())
    .def_static("New", &smtk::attribute::DateTimeItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DateTimeItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::DateTimeItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DateTimeItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::DateTimeItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::DateTimeItemDefinition::classname)
    .def("createCopy", &smtk::attribute::DateTimeItemDefinition::createCopy, py::arg("info"))
    .def("displayFormat", &smtk::attribute::DateTimeItemDefinition::displayFormat)
    .def("setDisplayFormat", &smtk::attribute::DateTimeItemDefinition::setDisplayFormat, py::arg("format"))
    .def("setEnableCalendarPopup", &smtk::attribute::DateTimeItemDefinition::setEnableCalendarPopup, py::arg("mode"))
    .def("setUseTimeZone", &smtk::attribute::DateTimeItemDefinition::setUseTimeZone, py::arg("mode"))
    .def("type", &smtk::attribute::DateTimeItemDefinition::type)
    .def("useCalendarPopup", &smtk::attribute::DateTimeItemDefinition::useCalendarPopup)
    .def("useTimeZone", &smtk::attribute::DateTimeItemDefinition::useTimeZone)

    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::DateTimeItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::DateTimeItemDefinition>(i);
      })

    ;
  return instance;
}

#endif
