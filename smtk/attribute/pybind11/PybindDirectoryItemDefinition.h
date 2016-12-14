//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_DirectoryItemDefinition_h
#define pybind_smtk_attribute_DirectoryItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DirectoryItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::DirectoryItemDefinition, smtk::attribute::FileSystemItemDefinition > pybind11_init_smtk_attribute_DirectoryItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DirectoryItemDefinition, smtk::attribute::FileSystemItemDefinition > instance(m, "DirectoryItemDefinition");
  instance
    .def(py::init<::smtk::attribute::DirectoryItemDefinition const &>())
    .def_static("New", &smtk::attribute::DirectoryItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DirectoryItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::DirectoryItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::DirectoryItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::DirectoryItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::DirectoryItemDefinition::classname)
    .def("createCopy", &smtk::attribute::DirectoryItemDefinition::createCopy, py::arg("info"))
    .def("type", &smtk::attribute::DirectoryItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::DirectoryItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
