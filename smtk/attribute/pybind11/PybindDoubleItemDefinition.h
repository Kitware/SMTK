//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_DoubleItemDefinition_h
#define pybind_smtk_attribute_DoubleItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DoubleItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::DoubleItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<double> > pybind11_init_smtk_attribute_DoubleItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DoubleItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<double> > instance(m, "DoubleItemDefinition");
  instance
    .def(py::init<::smtk::attribute::DoubleItemDefinition const &>())
    .def_static("New", &smtk::attribute::DoubleItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DoubleItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::DoubleItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DoubleItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::DoubleItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("createCopy", &smtk::attribute::DoubleItemDefinition::createCopy, py::arg("info"))
    .def("type", &smtk::attribute::DoubleItemDefinition::type)
    .def("setDefaultValue", (bool (smtk::attribute::DoubleItemDefinition::*)(double const &)) &smtk::attribute::DoubleItemDefinition::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::DoubleItemDefinition::*)(::std::vector<double, std::allocator<double> > const &)) &smtk::attribute::DoubleItemDefinition::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::DoubleItemDefinition::*)(double const &, const std::string&)) &smtk::attribute::DoubleItemDefinition::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::DoubleItemDefinition::*)(::std::vector<double, std::allocator<double> > const &, const std::string&)) &smtk::attribute::DoubleItemDefinition::setDefaultValue)
    .def("setDefaultValueAsString", (bool (smtk::attribute::DoubleItemDefinition::*)(const std::string&)) &smtk::attribute::DoubleItemDefinition::setDefaultValueAsString)
    .def("setDefaultValueAsString", (bool (smtk::attribute::DoubleItemDefinition::*)(::std::vector<std::string, std::allocator<std::string> > const &)) &smtk::attribute::DoubleItemDefinition::setDefaultValueAsString)
    .def("defaultValueAsString", &smtk::attribute::DoubleItemDefinition::defaultValueAsString)
    .def("defaultValuesAsStrings", &smtk::attribute::DoubleItemDefinition::defaultValuesAsStrings)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::DoubleItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
