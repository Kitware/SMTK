//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ReferenceItem_h
#define pybind_smtk_attribute_ReferenceItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ReferenceItem.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ReferenceItem, smtk::attribute::Item > pybind11_init_smtk_attribute_ReferenceItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ReferenceItem, smtk::attribute::Item > instance(m, "ReferenceItem");
  instance
    .def(py::init<::smtk::attribute::ReferenceItem const &>())
    .def("deepcopy", (smtk::attribute::ReferenceItem & (smtk::attribute::ReferenceItem::*)(::smtk::attribute::ReferenceItem const &)) &smtk::attribute::ReferenceItem::operator=)
    .def("appendObjectValue", &smtk::attribute::ReferenceItem::appendObjectValue, py::arg("val"))
    .def("assign", &smtk::attribute::ReferenceItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::ReferenceItem::begin)
    .def("contains", (bool (smtk::attribute::ReferenceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ReferenceItem::contains, py::arg("compId"))
    .def("contains", (bool (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::contains, py::arg("obj"))
    .def("definition", &smtk::attribute::ReferenceItem::definition)
    .def("end", &smtk::attribute::ReferenceItem::end)
    .def("find", (ptrdiff_t (smtk::attribute::ReferenceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ReferenceItem::find, py::arg("compId"))
    .def("find", (ptrdiff_t (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::find, py::arg("component"))
    .def("isExtensible", &smtk::attribute::ReferenceItem::isExtensible)
    .def("isSet", &smtk::attribute::ReferenceItem::isSet, py::arg("i") = 0)
    .def("lockType", &smtk::attribute::ReferenceItem::lockType)
    .def("maxNumberOfValues", &smtk::attribute::ReferenceItem::maxNumberOfValues)
    .def("numberOfRequiredValues", &smtk::attribute::ReferenceItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ReferenceItem::numberOfValues)
    .def("objectValue", (smtk::resource::PersistentObjectPtr (smtk::attribute::ReferenceItem::*)(std::size_t) const) &smtk::attribute::ReferenceItem::objectValue, py::arg("i") = 0)
    .def("removeValue", &smtk::attribute::ReferenceItem::removeValue, py::arg("i"))
    .def("reset", &smtk::attribute::ReferenceItem::reset)
    .def("setNumberOfValues", &smtk::attribute::ReferenceItem::setNumberOfValues, py::arg("newSize"))
    .def("setObjectValue", (bool (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&)) &smtk::attribute::ReferenceItem::setObjectValue, py::arg("val"))
    .def("setObjectValue", (bool (smtk::attribute::ReferenceItem::*)(::size_t, const ::smtk::resource::PersistentObjectPtr&)) &smtk::attribute::ReferenceItem::setObjectValue, py::arg("i"), py::arg("val"))
    .def("type", &smtk::attribute::ReferenceItem::type)
    .def("typeName", &smtk::attribute::ReferenceItem::typeName)
    .def("unset", &smtk::attribute::ReferenceItem::unset, py::arg("i") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ReferenceItem::*)() const) &smtk::attribute::ReferenceItem::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ReferenceItem::*)(::size_t) const) &smtk::attribute::ReferenceItem::valueAsString, py::arg("i"))
    ;
  return instance;
}

#endif
