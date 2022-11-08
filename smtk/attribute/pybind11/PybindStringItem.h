//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_StringItem_h
#define pybind_smtk_attribute_StringItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/StringItem.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::StringItem, smtk::attribute::ValueItemTemplate<std::basic_string<char> > > pybind11_init_smtk_attribute_StringItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::StringItem, smtk::attribute::ValueItemTemplate<std::basic_string<char> > > instance(m, "StringItem");
  instance
    .def(py::init<::smtk::attribute::StringItem const &>())
    .def("isSecure", &smtk::attribute::StringItem::isSecure)
    .def("type", &smtk::attribute::StringItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::StringItem>(i);
      })
    ;
  return instance;
}

#endif
