//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_attribute_DateTimeItem_h
#define pybind___smtk_attribute_DateTimeItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DateTimeItem.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::ValueItemTemplate<smtk::common::DateTimeZonePair> > pybind11_init_smtk_attribute_DateTimeItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::ValueItemTemplate<smtk::common::DateTimeZonePair> > instance(m, "DateTimeItem");
  instance
    .def(py::init<::smtk::attribute::DateTimeItem const &>())
    .def("assign", &smtk::attribute::DateTimeItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("classname", &smtk::attribute::DateTimeItem::classname)
    .def("type", &smtk::attribute::DateTimeItem::type)
    ;
  return instance;
}

#endif
