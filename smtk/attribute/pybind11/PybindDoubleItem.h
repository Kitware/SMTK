//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_DoubleItem_h
#define pybind_smtk_attribute_DoubleItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DoubleItem.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::DoubleItem, smtk::attribute::ValueItemTemplate<double> > pybind11_init_smtk_attribute_DoubleItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DoubleItem, smtk::attribute::ValueItemTemplate<double> > instance(m, "DoubleItem");
  instance
    .def("setValue", (bool (smtk::attribute::DoubleItem::*)(double const &)) &smtk::attribute::DoubleItem::setValue)
    .def("setValue", (bool (smtk::attribute::DoubleItem::*)(::size_t, double const &)) &smtk::attribute::DoubleItem::setValue)
    .def("setValue", (bool (smtk::attribute::DoubleItem::*)(::size_t, double const &, const std::string&)) &smtk::attribute::DoubleItem::setValue)
    .def(py::init<::smtk::attribute::DoubleItem const &>())
    .def("type", &smtk::attribute::DoubleItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(i);
      })
    ;
  return instance;
}

#endif
