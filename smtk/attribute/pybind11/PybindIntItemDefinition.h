//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_IntItemDefinition_h
#define pybind_smtk_attribute_IntItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/IntItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::IntItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<int> > pybind11_init_smtk_attribute_IntItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::IntItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<int> > instance(m, "IntItemDefinition");
  instance
    .def(py::init<::smtk::attribute::IntItemDefinition const &>())
    .def_static("New", &smtk::attribute::IntItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::IntItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::IntItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::IntItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::IntItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::IntItemDefinition::classname)
    .def("createCopy", &smtk::attribute::IntItemDefinition::createCopy, py::arg("info"))
    .def("type", &smtk::attribute::IntItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::IntItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
