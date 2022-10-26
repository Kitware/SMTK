//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ComponentItem_h
#define pybind_smtk_attribute_ComponentItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ComponentItem.h"

#include "smtk/attribute/ReferenceItem.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Component.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::ComponentItem, smtk::attribute::ReferenceItem > pybind11_init_smtk_attribute_ComponentItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ComponentItem, smtk::attribute::ReferenceItem > instance(m, "ComponentItem");
  instance
    .def(py::init<::smtk::attribute::ComponentItem const &>())
    .def("deepcopy", (smtk::attribute::ComponentItem & (smtk::attribute::ComponentItem::*)(::smtk::attribute::ComponentItem const &)) &smtk::attribute::ComponentItem::operator=)
    .def("appendValue", &smtk::attribute::ComponentItem::appendValue, py::arg("val"), py::arg("allowDuplicates") = true)
    .def("begin", &smtk::attribute::ComponentItem::begin)
    .def("contains", (bool (smtk::attribute::ComponentItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ComponentItem::contains, py::arg("compId"))
    .def("contains", (bool (smtk::attribute::ComponentItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ComponentItem::contains, py::arg("comp"))
    .def("definition", &smtk::attribute::ComponentItem::definition)
    .def("end", &smtk::attribute::ComponentItem::end)
    .def("_find", (smtk::attribute::ItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("_find", (smtk::attribute::ConstItemPtr (smtk::attribute::Item::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::Item::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("find", (ptrdiff_t (smtk::attribute::ComponentItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ComponentItem::find, py::arg("compId"))
    .def("find", (ptrdiff_t (smtk::attribute::ComponentItem::*)(const ::smtk::resource::PersistentObjectPtr&) const) &smtk::attribute::ComponentItem::find, py::arg("component"))
    .def("isExtensible", &smtk::attribute::ComponentItem::isExtensible)
    .def("isSet", &smtk::attribute::ComponentItem::isSet, py::arg("i") = 0)
    .def("numberOfRequiredValues", &smtk::attribute::ComponentItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ComponentItem::numberOfValues)
    .def("removeValue", &smtk::attribute::ComponentItem::removeValue, py::arg("i"))
    .def("reset", &smtk::attribute::ComponentItem::reset)
    .def("setNumberOfValues", &smtk::attribute::ComponentItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::ComponentItem::*)(::smtk::resource::ComponentPtr)) &smtk::attribute::ComponentItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::ComponentItem::*)(::size_t, ::smtk::resource::ComponentPtr)) &smtk::attribute::ComponentItem::setValue, py::arg("i"), py::arg("val"))
    .def("type", &smtk::attribute::ComponentItem::type)
    .def("unset", &smtk::attribute::ComponentItem::unset, py::arg("i") = 0)
    .def("value", &smtk::attribute::ComponentItem::value, py::arg("i") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ComponentItem::*)() const) &smtk::attribute::ComponentItem::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ComponentItem::*)(::size_t) const) &smtk::attribute::ComponentItem::valueAsString, py::arg("i"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(i);
      })
    ;
  return instance;
}

#endif
