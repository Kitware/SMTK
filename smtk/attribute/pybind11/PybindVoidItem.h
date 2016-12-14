//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_VoidItem_h
#define pybind_smtk_attribute_VoidItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/VoidItem.h"

#include "smtk/attribute/Item.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::VoidItem, smtk::attribute::Item > pybind11_init_smtk_attribute_VoidItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::VoidItem, smtk::attribute::Item > instance(m, "VoidItem");
  instance
    .def(py::init<::smtk::attribute::VoidItem const &>())
    .def("deepcopy", (smtk::attribute::VoidItem & (smtk::attribute::VoidItem::*)(::smtk::attribute::VoidItem const &)) &smtk::attribute::VoidItem::operator=)
    .def("classname", &smtk::attribute::VoidItem::classname)
    .def("isValid", &smtk::attribute::VoidItem::isValid)
    .def("type", &smtk::attribute::VoidItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::VoidItem>(i);
      })    ;
  return instance;
}

#endif
