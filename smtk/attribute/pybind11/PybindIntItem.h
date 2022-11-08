//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_IntItem_h
#define pybind_smtk_attribute_IntItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/IntItem.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::IntItem, smtk::attribute::ValueItemTemplate<int> > pybind11_init_smtk_attribute_IntItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::IntItem, smtk::attribute::ValueItemTemplate<int> > instance(m, "IntItem");
  instance
    .def(py::init<::smtk::attribute::IntItem const &>())
    .def("type", &smtk::attribute::IntItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::IntItem>(i);
      })
    ;
  return instance;
}

#endif
