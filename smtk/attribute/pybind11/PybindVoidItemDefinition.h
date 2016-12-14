//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_VoidItemDefinition_h
#define pybind_smtk_attribute_VoidItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::VoidItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_VoidItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::VoidItemDefinition, smtk::attribute::ItemDefinition > instance(m, "VoidItemDefinition");
  instance
    .def(py::init<::smtk::attribute::VoidItemDefinition const &>())
    .def_static("New", &smtk::attribute::VoidItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::VoidItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::VoidItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::VoidItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::VoidItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::VoidItemDefinition::classname)
    .def("createCopy", &smtk::attribute::VoidItemDefinition::createCopy, py::arg("info"))
    .def("type", &smtk::attribute::VoidItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::VoidItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
