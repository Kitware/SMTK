//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_RefItem_h
#define pybind_smtk_attribute_RefItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/RefItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::RefItem, smtk::attribute::Item > pybind11_init_smtk_attribute_RefItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::RefItem, smtk::attribute::Item > instance(m, "RefItem");
  instance
    .def(py::init<::smtk::attribute::RefItem const &>())
    .def("deepcopy", (smtk::attribute::RefItem & (smtk::attribute::RefItem::*)(::smtk::attribute::RefItem const &)) &smtk::attribute::RefItem::operator=)
    .def("appendValue", &smtk::attribute::RefItem::appendValue, py::arg("val"))
    .def("assign", &smtk::attribute::RefItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::RefItem::begin)
    .def("classname", &smtk::attribute::RefItem::classname)
    .def("end", &smtk::attribute::RefItem::end)
    .def("isSet", &smtk::attribute::RefItem::isSet, py::arg("element") = 0)
    .def("isValid", &smtk::attribute::RefItem::isValid)
    .def("numberOfRequiredValues", &smtk::attribute::RefItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::RefItem::numberOfValues)
    .def("removeValue", &smtk::attribute::RefItem::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::RefItem::reset)
    .def("setNumberOfValues", &smtk::attribute::RefItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::RefItem::*)(::smtk::attribute::AttributePtr)) &smtk::attribute::RefItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::RefItem::*)(::size_t, ::smtk::attribute::AttributePtr)) &smtk::attribute::RefItem::setValue, py::arg("element"), py::arg("val"))
    .def("type", &smtk::attribute::RefItem::type)
    .def("unset", &smtk::attribute::RefItem::unset, py::arg("element") = 0)
    .def("value", &smtk::attribute::RefItem::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::RefItem::*)(::std::string const &) const) &smtk::attribute::RefItem::valueAsString, py::arg("format") = "")
    .def("valueAsString", (std::string (smtk::attribute::RefItem::*)(::size_t, ::std::string const &) const) &smtk::attribute::RefItem::valueAsString, py::arg("element"), py::arg("format") = "")
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::RefItem>(i);
      })
    ;
  return instance;
}

#endif
