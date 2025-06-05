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

inline PySharedPtrClass< smtk::attribute::ReferenceItem, smtk::attribute::Item > pybind11_init_smtk_attribute_ReferenceItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ReferenceItem, smtk::attribute::Item > instance(m, "ReferenceItem");
  instance
    .def(py::init<::smtk::attribute::ReferenceItem const &>())
    .def("deepcopy", (smtk::attribute::ReferenceItem & (smtk::attribute::ReferenceItem::*)(::smtk::attribute::ReferenceItem const &)) &smtk::attribute::ReferenceItem::operator=)
    .def("_activeChildItem", &smtk::attribute::ReferenceItem::activeChildItem, py::arg("i"))
    .def("appendValue", &smtk::attribute::ReferenceItem::appendValue, py::arg("val"), py::arg("allowDuplicates") = true)
    .def("begin", &smtk::attribute::ReferenceItem::begin)
    .def("childrenItems", &smtk::attribute::ReferenceItem::childrenItems)
    .def("contains", (bool (smtk::attribute::ReferenceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ReferenceItem::contains, py::arg("compId"))
    .def("contains", (bool (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::contains, py::arg("obj"))
    .def("currentConditional", &smtk::attribute::ReferenceItem::currentConditional)
    .def("definition", &smtk::attribute::ReferenceItem::definition)
    .def("end", &smtk::attribute::ReferenceItem::end)
    .def("_find", (smtk::attribute::ItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("_find", (smtk::attribute::ConstItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("find", (ptrdiff_t (smtk::attribute::ReferenceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ReferenceItem::find, py::arg("compId"))
    .def("find", (ptrdiff_t (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::find, py::arg("component"))
    .def("isExtensible", &smtk::attribute::ReferenceItem::isExtensible)
    .def("isSet", &smtk::attribute::ReferenceItem::isSet, py::arg("i") = 0)
    .def("isValueValid", (bool (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::isValueValid, py::arg("val"))
    .def("isValueValid", (bool (smtk::attribute::ReferenceItem::*)(::size_t, const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ReferenceItem::isValueValid, py::arg("i"), py::arg("val"))
    .def("lockType", &smtk::attribute::ReferenceItem::lockType)
    .def("maxNumberOfValues", &smtk::attribute::ReferenceItem::maxNumberOfValues)
    .def("numberOfActiveChildrenItems", &smtk::attribute::ReferenceItem::numberOfActiveChildrenItems)
    .def("numberOfChildrenItems", &smtk::attribute::ReferenceItem::numberOfChildrenItems)
    .def("numberOfRequiredValues", &smtk::attribute::ReferenceItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ReferenceItem::numberOfValues)
    .def("value", (smtk::resource::PersistentObjectPtr (smtk::attribute::ReferenceItem::*)(std::size_t) const) &smtk::attribute::ReferenceItem::value, py::arg("i") = 0)
    .def("removeValue", &smtk::attribute::ReferenceItem::removeValue, py::arg("i"))
    .def("reset", &smtk::attribute::ReferenceItem::reset)
    .def("setNumberOfValues", &smtk::attribute::ReferenceItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::ReferenceItem::*)(const ::smtk::resource::PersistentObjectPtr&)) &smtk::attribute::ReferenceItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::ReferenceItem::*)(::size_t, const ::smtk::resource::PersistentObjectPtr&)) &smtk::attribute::ReferenceItem::setValue, py::arg("i"), py::arg("val"))
    .def("type", &smtk::attribute::ReferenceItem::type)
    .def("typeName", &smtk::attribute::ReferenceItem::typeName)
    .def("unset", &smtk::attribute::ReferenceItem::unset, py::arg("i") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ReferenceItem::*)() const) &smtk::attribute::ReferenceItem::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ReferenceItem::*)(::size_t) const) &smtk::attribute::ReferenceItem::valueAsString, py::arg("i"))
    .def("setValues", [&](smtk::attribute::ReferenceItem* self, const std::vector<smtk::resource::PersistentObject::Ptr>& values)
      {
        return self->setValues(values.begin(), values.end());
      }, py::arg("values"))
    .def("values", [&](smtk::attribute::ReferenceItem* self) -> std::vector<smtk::resource::PersistentObject::Ptr>
      {
        std::vector<smtk::resource::PersistentObject::Ptr> values;
        std::size_t nn = self->numberOfValues();
        values.reserve(nn);
        for (std::size_t ii = 0; ii < nn; ++ii)
        {
          values.push_back(self->isSet(ii) ? self->value(ii) : smtk::resource::PersistentObject::Ptr());
        }
        return values;
      })
    ;
  return instance;
}

#endif
