//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_DateTimeItem_h
#define pybind_smtk_attribute_DateTimeItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DateTimeItem.h"

#include "smtk/attribute/Item.h"
#include "smtk/common/DateTimeZonePair.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::Item > pybind11_init_smtk_attribute_DateTimeItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::Item > instance(m, "DateTimeItem");
  instance
    .def(py::init<::smtk::attribute::DateTimeItem const &>())
    .def("deepcopy", (smtk::attribute::DateTimeItem & (smtk::attribute::DateTimeItem::*)(::smtk::attribute::DateTimeItem const &)) &smtk::attribute::DateTimeItem::operator=)
    .def("isSet", &smtk::attribute::DateTimeItem::isSet, py::arg("element") = 0)
    .def("isUsingDefault", (bool (smtk::attribute::DateTimeItem::*)(::std::size_t) const) &smtk::attribute::DateTimeItem::isUsingDefault, py::arg("elementIndex"))
    .def("isUsingDefault", (bool (smtk::attribute::DateTimeItem::*)() const) &smtk::attribute::DateTimeItem::isUsingDefault)
    .def("hasDefault", &smtk::attribute::DateTimeItem::hasDefault)
    .def("numberOfRequiredValues", &smtk::attribute::DateTimeItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::DateTimeItem::numberOfValues)
    .def("reset", &smtk::attribute::DateTimeItem::reset)
    .def("setNumberOfValues", &smtk::attribute::DateTimeItem::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::DateTimeItem::setToDefault, py::arg("elementIndex") = 0)
    .def("setValue", (bool (smtk::attribute::DateTimeItem::*)(::smtk::common::DateTimeZonePair const &)) &smtk::attribute::DateTimeItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::DateTimeItem::*)(::std::size_t, ::smtk::common::DateTimeZonePair const &)) &smtk::attribute::DateTimeItem::setValue, py::arg("element"), py::arg("val"))
    .def("type", &smtk::attribute::DateTimeItem::type)
    .def("unset", &smtk::attribute::DateTimeItem::unset, py::arg("element") = 0)
    .def("value", &smtk::attribute::DateTimeItem::value, py::arg("element") = 0)
    ;
  return instance;
}

#endif
