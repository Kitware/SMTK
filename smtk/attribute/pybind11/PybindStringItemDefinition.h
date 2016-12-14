//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_StringItemDefinition_h
#define pybind_smtk_attribute_StringItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::StringItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<std::basic_string<char> > > pybind11_init_smtk_attribute_StringItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::StringItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<std::basic_string<char> > > instance(m, "StringItemDefinition");
  instance
    .def(py::init<::smtk::attribute::StringItemDefinition const &>())
    .def_static("New", &smtk::attribute::StringItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::StringItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::StringItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::StringItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::StringItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::StringItemDefinition::classname)
    .def("createCopy", &smtk::attribute::StringItemDefinition::createCopy, py::arg("info"))
    .def("isMultiline", &smtk::attribute::StringItemDefinition::isMultiline)
    .def("isSecure", &smtk::attribute::StringItemDefinition::isSecure)
    .def("setIsMultiline", &smtk::attribute::StringItemDefinition::setIsMultiline, py::arg("val"))
    .def("setIsSecure", &smtk::attribute::StringItemDefinition::setIsSecure, py::arg("val"))
    .def("type", &smtk::attribute::StringItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::StringItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
